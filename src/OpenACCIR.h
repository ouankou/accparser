#include <algorithm>
#include <cassert>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>

#include "OpenACCKinds.h"

struct OpenACCExpressionItem {
  std::string text;
  OpenACCClauseSeparator separator = ACCC_CLAUSE_SEP_comma;
};

enum OpenACCBaseLang {
  ACC_Lang_C,
  ACC_Lang_Cplusplus,
  ACC_Lang_Fortran,
  ACC_Lang_unknown
};

class ACC_SourceLocation {
  int line;
  int column;

  ACC_SourceLocation *parent_construct;

public:
  ACC_SourceLocation(int _line = 0, int _col = 0,
                     ACC_SourceLocation *_parent_construct = NULL)
      : line(_line), column(_col), parent_construct(_parent_construct){};
  void setParentConstruct(ACC_SourceLocation *_parent_construct) {
    parent_construct = _parent_construct;
  };
  ACC_SourceLocation *getParentConstruct() { return parent_construct; };
  int getLine() { return line; };
  void setLine(int _line) { line = _line; };
  int getColumn() { return column; };
  void setColumn(int _column) { column = _column; };
};

/**
 * The class or baseclass for all the clause classes. For all the clauses that
 * only take 0 to multiple expression or variables, we use this class to create
 * objects. For all other clauses, which requires at least one parameters, we
 * will have an inherit class from this one, and the superclass contains fields
 * for the parameters
 */
class OpenACCClause : public ACC_SourceLocation {
protected:
  OpenACCClauseKind kind;
  // the clause position in the vector of clauses in original order
  int clause_position = -1;

  // Store original keyword text for alias preservation (e.g., pcreate vs create)
  std::string original_keyword = "";

  /* consider this is a struct of array, i.e.
   * the expression/localtionLine/locationColumn are the same index are one
   * record for an expression and its location
   */
  std::vector<std::string> expressions;
  std::vector<OpenACCClauseSeparator> expression_separators;

  std::vector<ACC_SourceLocation> locations;

public:
  OpenACCClause(OpenACCClauseKind k, int _line = 0, int _col = 0)
      : ACC_SourceLocation(_line, _col), kind(k){};

  OpenACCClauseKind getKind() { return kind; };
  int getClausePosition() { return clause_position; };
  void setClausePosition(int _clause_position) {
    clause_position = _clause_position;
  };

  void setOriginalKeyword(std::string keyword) { original_keyword = keyword; };
  std::string getOriginalKeyword() { return original_keyword; };

  // a list of expressions or variables that are language-specific for the
  // clause, accparser does not parse them, instead, it only stores them as
  // strings
  void addLangExpr(std::string expression_string,
                   OpenACCClauseSeparator sep = ACCC_CLAUSE_SEP_comma,
                   int line = 0, int col = 0);

  std::vector<std::string> *getExpressions() { return &expressions; };
  const std::vector<OpenACCClauseSeparator> &getExpressionSeparators() const {
    return expression_separators;
  }

  virtual std::string toString();
  virtual ~OpenACCClause() = default;
  std::string expressionToString();
};

// Common base for clauses that carry a variable list.
class OpenACCVarListClause : public OpenACCClause {
protected:
  std::vector<OpenACCExpressionItem> vars;

public:
  OpenACCVarListClause(OpenACCClauseKind k, int _line = 0, int _col = 0)
      : OpenACCClause(k, _line, _col) {}

  void addVar(const std::string &expr,
              OpenACCClauseSeparator sep = ACCC_CLAUSE_SEP_comma) {
    vars.push_back(OpenACCExpressionItem{expr, sep});
  }
  void addVar(const OpenACCExpressionItem &item) { vars.push_back(item); }

  const std::vector<OpenACCExpressionItem> &getVars() const { return vars; }

  std::string varsToString() const {
    std::string out;
    for (size_t idx = 0; idx < vars.size(); ++idx) {
      if (idx > 0) {
        out += (vars[idx].separator == ACCC_CLAUSE_SEP_comma) ? ", " : " ";
      }
      out += vars[idx].text;
    }
    return out;
  }
};

/**
 * The class for all the OpenACC directives
 */
class OpenACCDirective : public ACC_SourceLocation {
protected:
  OpenACCDirectiveKind kind;
  OpenACCBaseLang lang;

  /* Flag to control whether clauses should be merged/normalized.
   * When true (default), multiple clauses of the same type are merged.
   * When false, clauses are preserved separately for exact round-trip parsing.
   */
  static bool enable_clause_merging;

  /* The vector is used to store the pointers of clauses in original order.
   * While unparsing, the generated pragma keeps the clauses in the same order
   * as the input. For example, #pragma omp parallel shared(a) private(b) is the
   * input. The unparsing won't switch the order of share and private clause.
   * Share clause is always the first.
   *
   * For the clauses that could be normalized, we always merge the second one to
   * the first one. Then the second one will be eliminated and not stored
   * anywhere.
   */
  std::vector<OpenACCClause *> *clauses_in_original_order =
      new std::vector<OpenACCClause *>();

  /* the map to store clauses of the directive, for each clause, we store a
   * vector of OpenACCClause objects since there could be multiple clause
   * objects for those clauses that take parameters, e.g. reduction clause
   *
   * for those clauses just take no parameters, but may take some variables or
   * expressions, we only need to have one OpenACCClause object, e.g. shared,
   * private.
   *
   * The design and use of this map should make sure that for any clause, we
   * should only have one OpenACCClause object for each instance of kind and
   * full parameters
   */
  std::map<OpenACCClauseKind, std::vector<OpenACCClause *> *> clauses;
  /**
   *
   * This method searches the clauses map to see whether one or more
   * OpenACCClause objects of the specified kind parameters exist in the
   * directive, if so it returns the objects that match.
   * @param kind clause kind
   * @param parameters clause parameters
   * @return
   */
  std::vector<OpenACCClause *> searchOpenACCClause(OpenACCClauseKind kind,
                                                   int num, int *parameters);

  /**
   * Search and add a clause of kind and parameters specified by the variadic
   * parameters. This should be the only call used to add an OpenACCClause
   * object.
   *
   * The method may simply create an OpenACCClause-subclassed object and return
   * it. In this way, normalization will be needed later on.
   *
   * Or the method may do the normalization while adding a clause.
   * it first searches the clauses map to see whether an OpenACCClause object
   * of the specified kind and parameters exists in the map. If so, it only
   * return that OpenACCClause object, otherwise, it should create a new
   * OpenACCClause object and insert in the map
   *
   * NOTE: if only partial parameters are provided as keys to search for a
   * clause, the function will only return the first one that matches. Thus, the
   * method should NOT be called with partial parameters of a specific clause
   * @param kind
   * @param parameters clause parameters, number of parameters should be
   * determined by the kind
   * @return
   */
  OpenACCClause *addOpenACCClause(OpenACCClauseKind kind, int *parameters);
  /**
   * normalize all the clause of a specific kind
   * @param kind
   * @return
   */
  void *normalizeClause(OpenACCClauseKind kind);

public:
  OpenACCDirective(OpenACCDirectiveKind k,
                   OpenACCBaseLang _lang = ACC_Lang_unknown, int _line = 0,
                   int _col = 0)
      : ACC_SourceLocation(_line, _col), kind(k), lang(_lang){};

  OpenACCDirectiveKind getKind() { return kind; };

  // Static methods to control clause merging behavior
  static void setClauseMerging(bool enable) { enable_clause_merging = enable; }
  static bool getClauseMerging() { return enable_clause_merging; }

  std::map<OpenACCClauseKind, std::vector<OpenACCClause *> *> *getAllClauses() {
    return &clauses;
  };

  std::vector<OpenACCClause *> *getClauses(OpenACCClauseKind kind) {
    return clauses[kind];
  };
  std::vector<OpenACCClause *> *getClausesInOriginalOrder() {
    return clauses_in_original_order;
  };

  virtual std::string toString();

  /* Ensure safe polymorphic destruction when callers delete a derived
   * directive via an OpenACCDirective* pointer (tests do this). */
  virtual ~OpenACCDirective() = default;

  std::string generatePragmaString(std::string _prefix = "#pragma acc ",
                                   std::string _beginning_symbol = "",
                                   std::string _ending_symbol = "");
  // To call this method directly to add new clause, it can't be protected.
  OpenACCClause *addOpenACCClause(int, ...);
  void setBaseLang(OpenACCBaseLang _lang) { lang = _lang; };
  OpenACCBaseLang getBaseLang() { return lang; };
};

// Cache directive
class OpenACCCacheDirective : public OpenACCDirective {
protected:
  OpenACCCacheDirectiveModifier modifier = ACCC_CACHE_unspecified;
  std::vector<std::string> vars;

public:
  OpenACCCacheDirective() : OpenACCDirective(ACCD_cache){};
  OpenACCCacheDirectiveModifier getModifier() { return modifier; };
  void setModifier(OpenACCCacheDirectiveModifier _modifier) {
    modifier = _modifier;
  };
  const std::vector<std::string> &getVars() const { return vars; }
  void addVar(const std::string &_string) {
    if (std::find(vars.begin(), vars.end(), _string) == vars.end()) {
      vars.push_back(_string);
    }
  };
  std::string varsToString() const {
    std::string out;
    for (auto it = vars.begin(); it != vars.end(); ++it) {
      out += *it;
      if (it + 1 != vars.end()) {
        out += ", ";
      }
    }
    return out;
  }
  std::string toString();
};

// End directive
class OpenACCEndDirective : public OpenACCDirective {
protected:
  OpenACCDirective *paired_directive;

public:
  OpenACCEndDirective() : OpenACCDirective(ACCD_end){};
  void setPairedDirective(OpenACCDirective *_paired_directive) {
    paired_directive = _paired_directive;
  };
  OpenACCDirective *getPairedDirective() { return paired_directive; };
};

// Routine directive
class OpenACCRoutineDirective : public OpenACCDirective {
protected:
  std::string name = "";
  bool name_is_string_literal = false;

public:
  OpenACCRoutineDirective() : OpenACCDirective(ACCD_routine){};
  void setName(std::string _name, bool is_string_literal = false) {
    name = _name;
    name_is_string_literal = is_string_literal;
  };
  std::string getName() { return name; };
  bool isNameStringLiteral() const { return name_is_string_literal; };
};

// Wait directive
class OpenACCWaitDirective : public OpenACCDirective {
protected:
  std::vector<std::string> async_ids;
  std::string devnum = "";
  bool queues = false;

public:
  OpenACCWaitDirective() : OpenACCDirective(ACCD_wait){};
  void setDevnum(std::string _devnum) { devnum = _devnum; };
  std::string getDevnum() { return devnum; };
  void setQueues(bool _queues) { queues = _queues; };
  bool getQueues() { return queues; };
  std::vector<std::string> *getAsyncIds() { return &async_ids; };
  void addAsyncId(std::string _string) { async_ids.push_back(_string); };
  std::string toString();
  std::string expressionToString();
};

// Async Clause
class OpenACCAsyncClause : public OpenACCClause {

protected:
  OpenACCAsyncModifier modifier = ACCC_ASYNC_unspecified;
  std::string async_expr;

public:
  OpenACCAsyncClause() : OpenACCClause(ACCC_async){};

  void setModifier(OpenACCAsyncModifier m) { modifier = m; }
  OpenACCAsyncModifier getModifier() const { return modifier; }
  void setAsyncExpr(const std::string &expr) { async_expr = expr; }
  const std::string &getAsyncExpr() const { return async_expr; }

  static OpenACCClause *addClause(OpenACCDirective *);
  std::string toString();
  void mergeClause(OpenACCDirective *, OpenACCClause *);
};

// Bind Clause
class OpenACCBindClause : public OpenACCClause {

protected:
  std::string binding = "";
  bool is_string_literal = false;

public:
  OpenACCBindClause() : OpenACCClause(ACCC_bind){};

  void setBinding(const std::string &_binding, bool _is_string_literal) {
    binding = _binding;
    is_string_literal = _is_string_literal;
  }
  const std::string &getBinding() const { return binding; }
  bool isStringLiteral() const { return is_string_literal; }

  static OpenACCClause *addClause(OpenACCDirective *);
  std::string toString();
  void mergeClause(OpenACCDirective *, OpenACCClause *);
};

// Collapse Clause
class OpenACCCollapseClause : public OpenACCClause {

protected:
  std::vector<std::string> counts;

public:
  OpenACCCollapseClause() : OpenACCClause(ACCC_collapse){};

  void addCountExpr(const std::string &expr) { counts.push_back(expr); }
  const std::vector<std::string> &getCounts() const { return counts; }

  static OpenACCClause *addClause(OpenACCDirective *);
  std::string toString();
  void mergeClause(OpenACCDirective *, OpenACCClause *);
};

// Copy Clause
class OpenACCCopyClause : public OpenACCVarListClause {

public:
  OpenACCCopyClause() : OpenACCVarListClause(ACCC_copy) {}

  static OpenACCClause *addClause(OpenACCDirective *);
  std::string toString();
  void mergeClause(OpenACCDirective *, OpenACCClause *);
};

// Copyin Clause
class OpenACCCopyinClause : public OpenACCVarListClause {

protected:
  OpenACCCopyinClauseModifier modifier = ACCC_COPYIN_unspecified;

public:
  OpenACCCopyinClause() : OpenACCVarListClause(ACCC_copyin){};

  OpenACCCopyinClauseModifier getModifier() { return modifier; };

  void setModifier(OpenACCCopyinClauseModifier _modifier) {
    modifier = _modifier;
  };

  static OpenACCClause *addClause(OpenACCDirective *);
  std::string toString();
  void mergeClause(OpenACCDirective *, OpenACCClause *);
};

// Copyout Clause
class OpenACCCopyoutClause : public OpenACCVarListClause {

protected:
  OpenACCCopyoutClauseModifier modifier = ACCC_COPYOUT_unspecified;

public:
  OpenACCCopyoutClause() : OpenACCVarListClause(ACCC_copyout){};

  OpenACCCopyoutClauseModifier getModifier() { return modifier; };

  void setModifier(OpenACCCopyoutClauseModifier _modifier) {
    modifier = _modifier;
  };

  static OpenACCClause *addClause(OpenACCDirective *);
  std::string toString();
  void mergeClause(OpenACCDirective *, OpenACCClause *);
};

// Create Clause
class OpenACCCreateClause : public OpenACCVarListClause {

protected:
  OpenACCCreateClauseModifier modifier = ACCC_CREATE_unspecified;

public:
  OpenACCCreateClause() : OpenACCVarListClause(ACCC_create){};

  OpenACCCreateClauseModifier getModifier() { return modifier; };

  void setModifier(OpenACCCreateClauseModifier _modifier) {
    modifier = _modifier;
  };

  static OpenACCClause *addClause(OpenACCDirective *);
  std::string toString();
  void mergeClause(OpenACCDirective *, OpenACCClause *);
};

// No_create Clause
class OpenACCNoCreateClause : public OpenACCVarListClause {
public:
  OpenACCNoCreateClause() : OpenACCVarListClause(ACCC_no_create) {}

  static OpenACCClause *addClause(OpenACCDirective *);
  std::string toString();
  void mergeClause(OpenACCDirective *, OpenACCClause *);
};

// Present Clause
class OpenACCPresentClause : public OpenACCVarListClause {
public:
  OpenACCPresentClause() : OpenACCVarListClause(ACCC_present) {}

  static OpenACCClause *addClause(OpenACCDirective *);
  std::string toString();
  void mergeClause(OpenACCDirective *, OpenACCClause *);
};

// Link Clause
class OpenACCLinkClause : public OpenACCVarListClause {
public:
  OpenACCLinkClause() : OpenACCVarListClause(ACCC_link) {}

  static OpenACCClause *addClause(OpenACCDirective *);
  std::string toString();
  void mergeClause(OpenACCDirective *, OpenACCClause *);
};

// Device Resident Clause
class OpenACCDeviceResidentClause : public OpenACCVarListClause {
public:
  OpenACCDeviceResidentClause()
      : OpenACCVarListClause(ACCC_device_resident) {}

  static OpenACCClause *addClause(OpenACCDirective *);
  std::string toString();
  void mergeClause(OpenACCDirective *, OpenACCClause *);
};

// Deviceptr Clause
class OpenACCDeviceptrClause : public OpenACCVarListClause {
public:
  OpenACCDeviceptrClause() : OpenACCVarListClause(ACCC_deviceptr) {}

  static OpenACCClause *addClause(OpenACCDirective *);
  std::string toString();
  void mergeClause(OpenACCDirective *, OpenACCClause *);
};

// Use_device Clause
class OpenACCUseDeviceClause : public OpenACCVarListClause {
public:
  OpenACCUseDeviceClause() : OpenACCVarListClause(ACCC_use_device) {}

  static OpenACCClause *addClause(OpenACCDirective *);
  std::string toString();
  void mergeClause(OpenACCDirective *, OpenACCClause *);
};

// Default Clause
class OpenACCDefaultClause : public OpenACCClause {

protected:
  OpenACCDefaultClauseKind default_kind = ACCC_DEFAULT_unspecified;

public:
  OpenACCDefaultClause() : OpenACCClause(ACCC_default){};

  OpenACCDefaultClauseKind getKind() { return default_kind; };

  void setKind(OpenACCDefaultClauseKind _default_kind) {
    default_kind = _default_kind;
  };

  static OpenACCClause *addClause(OpenACCDirective *);
  std::string toString();
};

// Default_async Clause
class OpenACCDefaultAsyncClause : public OpenACCClause {

protected:
  std::string async_expr;

public:
  OpenACCDefaultAsyncClause() : OpenACCClause(ACCC_default_async){};

  void setAsyncExpr(const std::string &expr) { async_expr = expr; }
  const std::string &getAsyncExpr() const { return async_expr; }

  static OpenACCClause *addClause(OpenACCDirective *);
  std::string toString();
  void mergeClause(OpenACCDirective *, OpenACCClause *);
};

// Attach Clause
class OpenACCAttachClause : public OpenACCVarListClause {
public:
  OpenACCAttachClause() : OpenACCVarListClause(ACCC_attach) {}

  static OpenACCClause *addClause(OpenACCDirective *);
  std::string toString();
  void mergeClause(OpenACCDirective *, OpenACCClause *);
};

// Delete Clause
class OpenACCDeleteClause : public OpenACCVarListClause {
public:
  OpenACCDeleteClause() : OpenACCVarListClause(ACCC_delete) {}

  static OpenACCClause *addClause(OpenACCDirective *);
  std::string toString();
  void mergeClause(OpenACCDirective *, OpenACCClause *);
};

// Detach Clause
class OpenACCDetachClause : public OpenACCVarListClause {
public:
  OpenACCDetachClause() : OpenACCVarListClause(ACCC_detach) {}

  static OpenACCClause *addClause(OpenACCDirective *);
  std::string toString();
  void mergeClause(OpenACCDirective *, OpenACCClause *);
};

// Device Clause
class OpenACCDeviceClause : public OpenACCClause {

protected:
  std::vector<std::string> devices;

public:
  OpenACCDeviceClause() : OpenACCClause(ACCC_device) {}

  void addDevice(const std::string &expr) {
    if (std::find(devices.begin(), devices.end(), expr) == devices.end()) {
      devices.push_back(expr);
    }
  }
  const std::vector<std::string> &getDevices() const { return devices; }

  static OpenACCClause *addClause(OpenACCDirective *);
  std::string toString();
  void mergeClause(OpenACCDirective *, OpenACCClause *);
};

// Firstprivate Clause
class OpenACCFirstprivateClause : public OpenACCVarListClause {
public:
  OpenACCFirstprivateClause() : OpenACCVarListClause(ACCC_firstprivate) {}

  static OpenACCClause *addClause(OpenACCDirective *);
  std::string toString();
  void mergeClause(OpenACCDirective *, OpenACCClause *);
};

// Host Clause
class OpenACCHostClause : public OpenACCVarListClause {
public:
  OpenACCHostClause() : OpenACCVarListClause(ACCC_host) {}

  static OpenACCClause *addClause(OpenACCDirective *);
  std::string toString();
  void mergeClause(OpenACCDirective *, OpenACCClause *);
};

// Private Clause
class OpenACCPrivateClause : public OpenACCVarListClause {
public:
  OpenACCPrivateClause() : OpenACCVarListClause(ACCC_private) {}

  static OpenACCClause *addClause(OpenACCDirective *);
  std::string toString();
  void mergeClause(OpenACCDirective *, OpenACCClause *);
};

// If Clause
class OpenACCIfClause : public OpenACCClause {

protected:
  std::string condition;

public:
  OpenACCIfClause() : OpenACCClause(ACCC_if) {}

  void setCondition(const std::string &expr) { condition = expr; }
  const std::string &getCondition() const { return condition; }

  static OpenACCClause *addClause(OpenACCDirective *);
  std::string toString();
  void mergeClause(OpenACCDirective *, OpenACCClause *);
};

// Gang Clause
class OpenACCGangClause : public OpenACCClause {

public:
  struct GangArg {
    OpenACCGangArgKind kind;
    std::string value;
  };

protected:
  std::vector<GangArg> args;

public:
  OpenACCGangClause() : OpenACCClause(ACCC_gang){};

  void addArg(OpenACCGangArgKind kind, const std::string &value) {
    args.push_back({kind, value});
  }
  const std::vector<GangArg> &getArgs() const { return args; }

  static OpenACCClause *addClause(OpenACCDirective *);
  std::string toString();
  void mergeClause(OpenACCDirective *, OpenACCClause *);
};

// Num_gangs Clause
class OpenACCNumGangsClause : public OpenACCClause {
protected:
  std::vector<std::string> nums;

public:
  OpenACCNumGangsClause() : OpenACCClause(ACCC_num_gangs){};

  void addNum(const std::string &expr) { nums.push_back(expr); }
  const std::vector<std::string> &getNums() const { return nums; }

  static OpenACCClause *addClause(OpenACCDirective *);
  std::string toString();
  void mergeClause(OpenACCDirective *, OpenACCClause *);
};

// Num_workers Clause
class OpenACCNumWorkersClause : public OpenACCClause {

protected:
  std::string num_expr;

public:
  OpenACCNumWorkersClause() : OpenACCClause(ACCC_num_workers){};

  void setNumExpr(const std::string &expr) { num_expr = expr; }
  const std::string &getNumExpr() const { return num_expr; }

  static OpenACCClause *addClause(OpenACCDirective *);
  std::string toString();
  void mergeClause(OpenACCDirective *, OpenACCClause *);
};

// Device_num Clause
class OpenACCDeviceNumClause : public OpenACCClause {
protected:
  std::string device_expr;

public:
  OpenACCDeviceNumClause() : OpenACCClause(ACCC_device_num) {}

  void setDeviceExpr(const std::string &expr) { device_expr = expr; }
  const std::string &getDeviceExpr() const { return device_expr; }

  static OpenACCClause *addClause(OpenACCDirective *);
  std::string toString();
  void mergeClause(OpenACCDirective *, OpenACCClause *);
};

// Tile Clause
class OpenACCTileClause : public OpenACCClause {
protected:
  std::vector<std::string> tile_sizes;

public:
  OpenACCTileClause() : OpenACCClause(ACCC_tile) {}

  void addTileSize(const std::string &expr) { tile_sizes.push_back(expr); }
  const std::vector<std::string> &getTileSizes() const { return tile_sizes; }

  static OpenACCClause *addClause(OpenACCDirective *);
  std::string toString();
  void mergeClause(OpenACCDirective *, OpenACCClause *);
};

// Reduction Clause
class OpenACCReductionClause : public OpenACCVarListClause {

protected:
  OpenACCReductionClauseOperator reduction_operator =
      ACCC_REDUCTION_unspecified;

public:
  OpenACCReductionClause() : OpenACCVarListClause(ACCC_reduction){};

  OpenACCReductionClauseOperator getOperator() { return reduction_operator; };

  void setOperator(OpenACCReductionClauseOperator _operator) {
    reduction_operator = _operator;
  };

  static OpenACCClause *addClause(OpenACCDirective *);
  std::string toString();
  void mergeClause(OpenACCDirective *, OpenACCClause *);
};

// Self Clause: supports either a condition or a variable list.
class OpenACCSelfClause : public OpenACCVarListClause {

protected:
  std::string condition;

public:
  OpenACCSelfClause() : OpenACCVarListClause(ACCC_self) {}

  void setCondition(const std::string &expr) { condition = expr; }
  const std::string &getCondition() const { return condition; }

  static OpenACCClause *addClause(OpenACCDirective *);
  std::string toString();
  void mergeClause(OpenACCDirective *, OpenACCClause *);
};

// Vector Clause
class OpenACCVectorClause : public OpenACCClause {
protected:
  OpenACCVectorClauseModifier modifier = ACCC_VECTOR_unspecified;
  std::string length_expr;

public:
  OpenACCVectorClause() : OpenACCClause(ACCC_vector){};

  OpenACCVectorClauseModifier getModifier() const { return modifier; };

  void setModifier(OpenACCVectorClauseModifier _modifier) {
    modifier = _modifier;
  };
  void setLengthExpr(const std::string &expr) { length_expr = expr; }
  const std::string &getLengthExpr() const { return length_expr; }

  static OpenACCClause *addClause(OpenACCDirective *);
  std::string toString();
  void mergeClause(OpenACCDirective *, OpenACCClause *);
};

// Vector_length Clause
class OpenACCVectorLengthClause : public OpenACCClause {

protected:
  std::string length_expr;

public:
  OpenACCVectorLengthClause() : OpenACCClause(ACCC_vector_length){};

  void setLengthExpr(const std::string &expr) { length_expr = expr; }
  const std::string &getLengthExpr() const { return length_expr; }

  static OpenACCClause *addClause(OpenACCDirective *);
  std::string toString();
  void mergeClause(OpenACCDirective *, OpenACCClause *);
};

// Wait Clause
class OpenACCWaitClause : public OpenACCClause {

protected:
  std::string devnum = "";
  bool queues = false;
  std::vector<std::string> async_ids;

public:
  OpenACCWaitClause() : OpenACCClause(ACCC_wait){};

  void setDevnum(std::string _devnum) { devnum = _devnum; };
  std::string getDevnum() { return devnum; };
  void setQueues(bool _queues) { queues = _queues; };
  bool getQueues() { return queues; };
  void addAsyncId(const std::string &expr) { async_ids.push_back(expr); }
  const std::vector<std::string> &getAsyncIds() const { return async_ids; }

  static OpenACCClause *addClause(OpenACCDirective *);
  std::string toString();
  void mergeClause(OpenACCDirective *, OpenACCClause *);
};

// Device_type Clause
class OpenACCDeviceTypeClause : public OpenACCClause {
protected:
  std::vector<OpenACCDeviceTypeKind> device_types;
  std::vector<std::string> unknown_types;

public:
  OpenACCDeviceTypeClause() : OpenACCClause(ACCC_device_type) {}

  void addDeviceType(OpenACCDeviceTypeKind kind);
  void addDeviceTypeString(const std::string &value);
  const std::vector<OpenACCDeviceTypeKind> &getDeviceTypes() const {
    return device_types;
  }
  const std::vector<std::string> &getUnknownDeviceTypes() const {
    return unknown_types;
  }
  static OpenACCClause *addClause(OpenACCDirective *);
  std::string toString();
  void mergeClause(OpenACCDirective *, OpenACCClause *);
};

// Worker Clause
class OpenACCWorkerClause : public OpenACCClause {

protected:
  OpenACCWorkerClauseModifier modifier = ACCC_WORKER_unspecified;
  std::string num_expr;

public:
  OpenACCWorkerClause() : OpenACCClause(ACCC_worker){};

  OpenACCWorkerClauseModifier getModifier() const { return modifier; };

  void setModifier(OpenACCWorkerClauseModifier _modifier) {
    modifier = _modifier;
  };
  void setNumExpr(const std::string &expr) { num_expr = expr; }
  const std::string &getNumExpr() const { return num_expr; }

  static OpenACCClause *addClause(OpenACCDirective *);
  std::string toString();
  void mergeClause(OpenACCDirective *, OpenACCClause *);
};
