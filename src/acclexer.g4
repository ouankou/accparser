lexer grammar acclexer;
// These are all supported lexer sections:

// Lexer file header. Appears at the top of h + cpp files. Use e.g. for copyrights.

@ lexer :: header
{ /* lexer header section */
}
// Appears before any #include in h + cpp files.

@ lexer :: preinclude
{ /* lexer precinclude section */
}
// Follows directly after the standard #includes in h + cpp files.

@ lexer :: postinclude
{
/* lexer postinclude section */
}
// Directly preceds the lexer class declaration in the h file (e.g. for additional types etc.).

@ lexer :: context
{ /* lexer context section */
}
// Appears in the public part of the lexer in the h file.

@ lexer :: members
{ /* public lexer declarations section */
  bool lookAhead(std::string word) {
    size_t i;
    for (i = 0; i < word.size(); i++) {
      if (_input->LA(i + 1) != (size_t)word[i]) {
        return false;
      }
    }
    return true;
  }
  bool lookAheadKWArgs(std::string keyword) {
    size_t i;
    for (i = 0; i < keyword.size(); i++) {
        if (_input->LA(i + 1) != (size_t)keyword[i]) {
            return false;
        }
    }
    size_t offset = keyword.size() + 1;
    while (true) {
        size_t nextChar = _input->LA(offset);
        if (nextChar == ' ' || nextChar == '\t' || nextChar == '\r' || nextChar == '\n') {
            offset++;
        } else {
            return nextChar == ':';
        }
    }
  }
}
// Appears in the private part of the lexer in the h file.

@ lexer :: declarations
{ /* private lexer declarations/members section */
  int parenthesis_local_count = 0;
  int parenthesis_global_count = 0;
  int bracket_count = 0;
  int colon_count = 0;
  bool paren_processed = false;
  bool data_clause_in_modifier_list = false;
  bool gang_clause_allows_keyword_args = false;
}
// Appears in line with the other class member definitions in the cpp file.

@ lexer :: definitions
{ /* lexer definitions section */
}
channels { CommentsChannel , DirectiveChannel }
tokens { EXPR }
C_PREFIX
   : [\p{White_Space}]* '#' [\p{White_Space}]* 'pragma'
   ;

FORTRAN_PREFIX
   : [\p{White_Space}]* [!c*] '$'
   ;

ACC
   : 'acc'
   ;

ATOMIC
   : 'atomic'
   ;

CACHE
   : 'cache' -> pushMode (cache_directive)
   ;

DATA
   : 'data'
   ;

END
   : 'end'
   ;

DECLARE
   : 'declare'
   ;

ENTER
   : 'enter'
   ;

EXIT
   : 'exit'
   ;

HOST_DATA
   : 'host_data'
   ;

INIT
   : 'init'
   ;

KERNELS
   : 'kernels'
   ;

LOOP
   : 'loop'
   ;

PARALLEL
   : 'parallel'
   ;

ROUTINE
   : 'routine' [\p{White_Space}]*
   {
  if (_input->LA(1) == '(') {
    pushMode(routine_directive);
  }
}
   ;

SERIAL
   : 'serial'
   ;

SET
   : 'set'
   ;

SHUTDOWN
   : 'shutdown'
   ;

UPDATE
   : 'update'
   ;

LEFT_PAREN
   : '('
   ;

RIGHT_PAREN
   : ')'
   ;

D_BLANK
   : [\p{White_Space}]+ -> skip
   ;

COLON
   : ':'
   ;

LINE_END
   : [\n\r] -> skip
   ;

CLAUSE_COMMA
   : ',' [\p{White_Space}]*
   -> skip
   ;

ASYNC
   : 'async' [\p{White_Space}]*
   {
  if (_input->LA(1) == '(') {
    pushMode(expr_clause);
  }
}
   ;

ATTACH
   : 'attach' -> pushMode (expr_clause)
   ;

AUTO
   : 'auto'
   ;

ALWAYS
   : 'always'
   ;

ALWAYSIN
   : 'alwaysin'
   ;

ALWAYSOUT
   : 'alwaysout'
   ;

FORCE
   : 'force'
   ;

BIND
   : 'bind' [\p{White_Space}]*
   {
  if (_input->LA(1) == '(') {
    pushMode(bind_clause);
  }
}
   ;

CAPTURE
   : 'capture'
   ;

COLLAPSE
   : 'collapse' -> pushMode (collapse_clause)
   ;

PCOPY
   : 'pcopy' -> pushMode (copy_clause)
   ;

PRESENT_OR_COPY
   : 'present_or_copy' -> pushMode (copy_clause)
   ;

COPY
   : 'copy' -> pushMode (copy_clause)
   ;

PCOPYIN
   : 'pcopyin' -> pushMode (copyin_clause)
   ;

PRESENT_OR_COPYIN
   : 'present_or_copyin' -> pushMode (copyin_clause)
   ;

COPYIN
   : 'copyin' -> pushMode (copyin_clause)
   ;

PCOPYOUT
   : 'pcopyout' -> pushMode (copyout_clause)
   ;

PRESENT_OR_COPYOUT
   : 'present_or_copyout' -> pushMode (copyout_clause)
   ;

COPYOUT
   : 'copyout' -> pushMode (copyout_clause)
   ;

PCREATE
   : 'pcreate' -> pushMode (create_clause)
   ;

PRESENT_OR_CREATE
   : 'present_or_create' -> pushMode (create_clause)
   ;

CREATE
   : 'create' -> pushMode (create_clause)
   ;

DEFAULT
   : 'default' -> pushMode (default_clause)
   ;

DEFAULT_ASYNC
   : 'default_async' -> pushMode (expr_clause)
   ;

DELETE
   : 'delete' -> pushMode (expr_clause)
   ;

DETACH
   : 'detach' -> pushMode (expr_clause)
   ;

DEVICE
   : 'device' -> pushMode (expr_clause)
   ;

DEVICE_NUM
   : 'device_num' -> pushMode (expr_clause)
   ;

DEVICE_RESIDENT
   : 'device_resident' -> pushMode (expr_clause)
   ;

DEVICE_TYPE
   : 'device_type' -> pushMode (device_type_clause)
   ;

DTYPE
   : 'dtype' -> type (DEVICE_TYPE) , pushMode (device_type_clause)
   ;

DEVICEPTR
   : 'deviceptr' -> pushMode (expr_clause)
   ;

FINALIZE
   : 'finalize'
   ;

FIRSTPRIVATE
   : 'firstprivate' -> pushMode (expr_clause)
   ;

GANG
   : 'gang' [\p{White_Space}]*
   {
  if (_input->LA(1) == '(') {
    gang_clause_allows_keyword_args = true;
    pushMode(gang_clause);
  }
}
   ;

HOST
   : 'host' -> pushMode (expr_clause)
   ;

IF
   : 'if' [\p{White_Space}]* -> pushMode (expr_clause)
   ;

IF_PRESENT
   : 'if_present'
   ;

INDEPENDENT
   : 'independent'
   ;

LINK
   : 'link' -> pushMode (expr_clause)
   ;

INDIRECT
   : 'indirect' [\p{White_Space}]*
   {
      if (_input->LA(1) == '(') {
          pushMode(expr_clause);
      }
   }
   ;

NOHOST
   : 'nohost'
   ;

NO_CREATE
   : 'no_create' -> pushMode (expr_clause)
   ;

NUM
   : 'num'
   ;

NUM_GANGS
   : 'num_gangs'
   {
  // Uses the same lexer mode as gang(...) but only carries expressions.
  gang_clause_allows_keyword_args = false;
}
     -> pushMode (gang_clause)
   ;

NUM_WORKERS
   : 'num_workers' -> pushMode (expr_clause)
   ;

PRESENT
   : 'present' -> pushMode (expr_clause)
   ;

PRIVATE
   : 'private' -> pushMode (expr_clause)
   ;

READ
   : 'read'
   ;

REDUCTION
   : 'reduction' -> pushMode (reduction_clause)
   ;

SELF
   : 'self' [\p{White_Space}]*
   {
  if (_input->LA(1) == '(')
    pushMode(expr_clause);
}
   ;

SEQ
   : 'seq'
   ;

TILE
   : 'tile'
   {
  // Uses the same lexer mode as gang(...) but only carries expressions.
  gang_clause_allows_keyword_args = false;
}
     -> pushMode (gang_clause)
   ;

USE_DEVICE
   : 'use_device' -> pushMode (expr_clause)
   ;

VECTOR
   : 'vector' [\p{White_Space}]*
   {
  if (_input->LA(1) == '(')
    pushMode(vector_clause);
}
   ;

VECTOR_LENGTH
   : 'vector_length' -> pushMode (expr_clause)
   ;

WAIT
   : 'wait' [\p{White_Space}]*
   {
  if (_input->LA(1) == '(')
    pushMode(wait_clause);
}
   ;

WORKER
   : 'worker' [\p{White_Space}]*
   {
  if (_input->LA(1) == '(')
    pushMode(worker_clause);
}
   ;

WRITE
   : 'write'
   ;

COMMENT
   : '//' ~ [\r\n]* -> skip
   ;

mode cache_directive;
CACHE_LEFT_PAREN
   : '(' [\p{White_Space}]*
   {
  setType(LEFT_PAREN);
  parenthesis_global_count = 1;
  parenthesis_local_count = 0;
  if (lookAhead("readonly") == false) {
    colon_count = 0;
    pushMode(expression_mode);
  }
}
   ;

CACHE_RIGHT_PAREN
   : ')' -> type (RIGHT_PAREN) , popMode
   ;

CACHE_READONLY
   : 'readonly' [\p{White_Space}]*
   {
  setType(READONLY);
  if ((_input->LA(1) == ':' && _input->LA(2) == ':') ||
      (_input->LA(1) != ':')) {
    colon_count = 1;
    more();
    pushMode(expression_mode);
  }
}
   ;

CACHE_COLON
   : ':' [\p{White_Space}]*
   {
  if (_input->LA(1) == ':')
    more();
  else {
    colon_count = 0;
    setType(COLON);
    pushMode(expression_mode);
  }
}
   ;

CACHE_COMMA
   : ',' [\p{White_Space}]*
   {
  skip();
  pushMode(expression_mode);
  parenthesis_global_count = 1;
  parenthesis_local_count = 0;
  colon_count = 0;
}
   ;

CACHE_BLANK
   : [\p{White_Space}]+ -> skip
   ;

CACHE_LINE_END
   : [\n\r] -> skip
   ;

mode routine_directive;
ROUTINE_LEFT_PAREN
   : '(' [\p{White_Space}]*
   {
  setType(LEFT_PAREN);
  parenthesis_global_count = 1;
  parenthesis_local_count = 0;
  pushMode(expression_mode);
}
   ;

ROUTINE_RIGHT_PAREN
   : ')' -> type (RIGHT_PAREN) , popMode
   ;

ROUTINE_BLANK
   : [\p{White_Space}]+ -> skip
   ;

ROUTINE_LINE_END
   : [\n\r] -> skip
   ;

mode copy_clause;
COPY_LEFT_PAREN
   : '(' [\p{White_Space}]*
   {
  setType(LEFT_PAREN);
  parenthesis_global_count = 1;
  parenthesis_local_count = 0;
  data_clause_in_modifier_list =
      lookAhead("always") || lookAhead("alwaysin") || lookAhead("alwaysout") ||
      lookAhead("capture") || lookAhead("readonly") || lookAhead("zero");
  if (!data_clause_in_modifier_list) {
    bracket_count = 0;
    colon_count = 1;
    pushMode(expression_mode);
  }
}
   ;

COPY_RIGHT_PAREN
   : ')'
   {
  data_clause_in_modifier_list = false;
}
     -> type (RIGHT_PAREN) , popMode
   ;

COPY_COLON
   : ':' [\p{White_Space}]*
   {
  setType(COLON);
  data_clause_in_modifier_list = false;
  bracket_count = 0;
  colon_count = 1;
  pushMode(expression_mode);
  parenthesis_global_count = 1;
  parenthesis_local_count = 0;
}
   ;

COPY_COMMA
   : ',' [\p{White_Space}]*
   {
  skip();
  if (!data_clause_in_modifier_list) {
    bracket_count = 0;
    colon_count = 1;
    pushMode(expression_mode);
    parenthesis_global_count = 1;
    parenthesis_local_count = 0;
  }
}
   ;

COPY_ALWAYSIN
   : 'alwaysin' -> type(ALWAYSIN)
   ;

COPY_ALWAYSOUT
   : 'alwaysout' -> type(ALWAYSOUT)
   ;

COPY_ALWAYS
   : 'always' -> type(ALWAYS)
   ;

COPY_CAPTURE
   : 'capture' -> type(CAPTURE)
   ;

COPY_READONLY
   : 'readonly' -> type(READONLY)
   ;

COPY_ZERO
   : 'zero' -> type(ZERO)
   ;

COPY_BLANK
   : [\p{White_Space}]+ -> skip
   ;

COPY_LINE_END
   : [\n\r] -> skip
   ;

mode copyin_clause;
COPYIN_LEFT_PAREN
   : '(' [\p{White_Space}]*
   {
  setType(LEFT_PAREN);
  parenthesis_global_count = 1;
  parenthesis_local_count = 0;
  data_clause_in_modifier_list =
      lookAhead("always") || lookAhead("alwaysin") || lookAhead("alwaysout") ||
      lookAhead("capture") || lookAhead("readonly") || lookAhead("zero");
  if (!data_clause_in_modifier_list) {
    bracket_count = 0;
    colon_count = 1;
    pushMode(expression_mode);
  }
}
   ;

COPYIN_RIGHT_PAREN
   : ')'
   {
  data_clause_in_modifier_list = false;
}
     -> type (RIGHT_PAREN) , popMode
   ;

READONLY
   : 'readonly'
   ;

COPYIN_ALWAYSIN
   : 'alwaysin' -> type(ALWAYSIN)
   ;

COPYIN_ALWAYSOUT
   : 'alwaysout' -> type(ALWAYSOUT)
   ;

COPYIN_ALWAYS
   : 'always' -> type(ALWAYS)
   ;

COPYIN_CAPTURE
   : 'capture' -> type(CAPTURE)
   ;

COPYIN_ZERO
   : 'zero' -> type(ZERO)
   ;

COPYIN_COLON
   : ':' [\p{White_Space}]*
   {
  setType(COLON);
  data_clause_in_modifier_list = false;
  bracket_count = 0;
  colon_count = 1;
  pushMode(expression_mode);
  parenthesis_global_count = 1;
  parenthesis_local_count = 0;
}
   ;

COPYIN_COMMA
   : ',' [\p{White_Space}]*
   {
  skip();
  if (!data_clause_in_modifier_list) {
    bracket_count = 0;
    colon_count = 1;
    pushMode(expression_mode);
    parenthesis_global_count = 1;
    parenthesis_local_count = 0;
  }
}
   ;

COPYIN_BLANK
   : [\p{White_Space}]+ -> skip
   ;

COPYIN_LINE_END
   : [\n\r] -> skip
   ;

mode copyout_clause;
COPYOUT_LEFT_PAREN
   : '(' [\p{White_Space}]*
   {
  setType(LEFT_PAREN);
  parenthesis_global_count = 1;
  parenthesis_local_count = 0;
  data_clause_in_modifier_list =
      lookAhead("always") || lookAhead("alwaysin") || lookAhead("alwaysout") ||
      lookAhead("capture") || lookAhead("readonly") || lookAhead("zero");
  if (!data_clause_in_modifier_list) {
    bracket_count = 0;
    colon_count = 1;
    pushMode(expression_mode);
  }
}
   ;

COPYOUT_RIGHT_PAREN
   : ')'
   {
  data_clause_in_modifier_list = false;
}
     -> type (RIGHT_PAREN) , popMode
   ;

ZERO
   : 'zero'
   ;

COPYOUT_ALWAYSIN
   : 'alwaysin' -> type(ALWAYSIN)
   ;

COPYOUT_ALWAYSOUT
   : 'alwaysout' -> type(ALWAYSOUT)
   ;

COPYOUT_ALWAYS
   : 'always' -> type(ALWAYS)
   ;

COPYOUT_CAPTURE
   : 'capture' -> type(CAPTURE)
   ;

COPYOUT_READONLY
   : 'readonly' -> type(READONLY)
   ;

COPYOUT_COLON
   : ':' [\p{White_Space}]*
   {
  setType(COLON);
  data_clause_in_modifier_list = false;
  bracket_count = 0;
  colon_count = 1;
  pushMode(expression_mode);
  parenthesis_global_count = 1;
  parenthesis_local_count = 0;
}
   ;

COPYOUT_COMMA
   : ',' [\p{White_Space}]*
   {
  skip();
  if (!data_clause_in_modifier_list) {
    bracket_count = 0;
    colon_count = 1;
    pushMode(expression_mode);
    parenthesis_global_count = 1;
    parenthesis_local_count = 0;
  }
}
   ;

COPYOUT_BLANK
   : [\p{White_Space}]+ -> skip
   ;

COPYOUT_LINE_END
   : [\n\r] -> skip
   ;

mode create_clause;
CREATE_LEFT_PAREN
   : '(' [\p{White_Space}]*
   {
  setType(LEFT_PAREN);
  parenthesis_global_count = 1;
  parenthesis_local_count = 0;
  data_clause_in_modifier_list =
      lookAhead("always") || lookAhead("alwaysin") || lookAhead("alwaysout") ||
      lookAhead("capture") || lookAhead("readonly") || lookAhead("zero");
  if (!data_clause_in_modifier_list) {
    bracket_count = 0;
    colon_count = 1;
    pushMode(expression_mode);
  }
}
   ;

CREATE_RIGHT_PAREN
   : ')'
   {
  data_clause_in_modifier_list = false;
}
     -> type (RIGHT_PAREN) , popMode
   ;

CREATE_ZERO
   : 'zero' -> type(ZERO)
   ;

CREATE_ALWAYSIN
   : 'alwaysin' -> type(ALWAYSIN)
   ;

CREATE_ALWAYSOUT
   : 'alwaysout' -> type(ALWAYSOUT)
   ;

CREATE_ALWAYS
   : 'always' -> type(ALWAYS)
   ;

CREATE_CAPTURE
   : 'capture' -> type(CAPTURE)
   ;

CREATE_READONLY
   : 'readonly' -> type(READONLY)
   ;

CREATE_COLON
   : ':' [\p{White_Space}]*
   {
  setType(COLON);
  data_clause_in_modifier_list = false;
  bracket_count = 0;
  colon_count = 1;
  pushMode(expression_mode);
  parenthesis_global_count = 1;
  parenthesis_local_count = 0;
}
   ;

CREATE_COMMA
   : ',' [\p{White_Space}]*
   {
  skip();
  if (!data_clause_in_modifier_list) {
    bracket_count = 0;
    colon_count = 1;
    pushMode(expression_mode);
    parenthesis_global_count = 1;
    parenthesis_local_count = 0;
  }
}
   ;

CREATE_BLANK
   : [\p{White_Space}]+ -> skip
   ;

CREATE_LINE_END
   : [\n\r] -> skip
   ;

mode default_clause;
NONE
   : 'none'
   ;

DEFAULT_PRESENT
   : 'present' -> type (PRESENT)
   ;

DEFAULT_LEFT_PAREN
   : '(' -> type (LEFT_PAREN)
   ;

DEFAULT_RIGHT_PAREN
   : ')' -> type (RIGHT_PAREN) , popMode
   ;

BLANK
   : [\p{White_Space}]+ -> skip
   ;

EXPR_LINE_END
   : [\n\r] -> skip
   ;

mode collapse_clause;
COLLAPSE_LEFT_PAREN
   : '(' [\p{White_Space}]*
   {
  setType(LEFT_PAREN);
  parenthesis_global_count = 1;
  parenthesis_local_count = 0;
  if (lookAhead("force") == false) {
    bracket_count = 0;
    colon_count = 1;
    pushMode(expression_mode);
  }
}
   ;

COLLAPSE_RIGHT_PAREN
   : ')' -> type (RIGHT_PAREN) , popMode
   ;

COLLAPSE_FORCE
   : 'force' -> type(FORCE)
   ;

COLLAPSE_COLON
   : ':' [\p{White_Space}]*
   {
  setType(COLON);
  bracket_count = 0;
  colon_count = 1;
  pushMode(expression_mode);
  parenthesis_global_count = 1;
  parenthesis_local_count = 0;
}
   ;

COLLAPSE_BLANK
   : [\p{White_Space}]+ -> skip
   ;

COLLAPSE_LINE_END
   : [\n\r] -> skip
   ;

mode vector_clause;
VECTOR_LEFT_PAREN
   : '(' [\p{White_Space}]*
   {
  setType(LEFT_PAREN);
  parenthesis_global_count = 1;
  parenthesis_local_count = 0;
  if (lookAhead("length") == false) {
    colon_count = 0;
    pushMode(expression_mode);
  }
}
   ;

VECTOR_RIGHT_PAREN
   : ')' -> type (RIGHT_PAREN) , popMode
   ;

LENGTH
   : 'length' [\p{White_Space}]*
   {
  if ((_input->LA(1) == ':' && _input->LA(2) == ':') ||
      (_input->LA(1) != ':')) {
    colon_count = 1;
    more();
    pushMode(expression_mode);
  }
}
   ;

VECTOR_COLON
   : ':' [\p{White_Space}]*
   {
  if (_input->LA(1) == ':')
    more();
  else {
    colon_count = 0;
    setType(COLON);
    pushMode(expression_mode);
  }
}
   ;

VECTOR_BLANK
   : [\p{White_Space}]+ -> skip
   ;

VECTOR_LINE_END
   : [\n\r] -> skip
   ;

mode reduction_clause;
REDUCTION_LEFT_PAREN
   : '(' [\p{White_Space}]*
   {
  setType(LEFT_PAREN);
  parenthesis_global_count = 1;
  parenthesis_local_count = 0;
}
   ;

REDUCTION_RIGHT_PAREN
   : ')' -> type (RIGHT_PAREN) , popMode
   ;

ADD
   : '+' [\p{White_Space}]*
   {
  if ((_input->LA(1) == ':' && _input->LA(2) == ':') ||
      (_input->LA(1) != ':')) {
    colon_count = 1;
    more();
    pushMode(expression_mode);
  }
}
   ;

SUB
   : '-' [\p{White_Space}]*
   {
  if ((_input->LA(1) == ':' && _input->LA(2) == ':') ||
      (_input->LA(1) != ':')) {
    colon_count = 1;
    more();
    pushMode(expression_mode);
  }
}
   ;

MUL
   : '*' [\p{White_Space}]*
   {
  if ((_input->LA(1) == ':' && _input->LA(2) == ':') ||
      (_input->LA(1) != ':')) {
    colon_count = 1;
    more();
    pushMode(expression_mode);
  }
}
   ;

MAX
   : 'max' [\p{White_Space}]*
   {
  if ((_input->LA(1) == ':' && _input->LA(2) == ':') ||
      (_input->LA(1) != ':')) {
    colon_count = 1;
    more();
    pushMode(expression_mode);
  }
}
   ;

MIN
   : 'min' [\p{White_Space}]*
   {
  if ((_input->LA(1) == ':' && _input->LA(2) == ':') ||
      (_input->LA(1) != ':')) {
    colon_count = 1;
    more();
    pushMode(expression_mode);
  }
}
   ;

BITAND
   : '&' [\p{White_Space}]*
   {
  if ((_input->LA(1) == ':' && _input->LA(2) == ':') ||
      (_input->LA(1) != ':')) {
    colon_count = 1;
    more();
    pushMode(expression_mode);
  }
}
   ;

BITOR
   : '|' [\p{White_Space}]*
   {
  if ((_input->LA(1) == ':' && _input->LA(2) == ':') ||
      (_input->LA(1) != ':')) {
    colon_count = 1;
    more();
    pushMode(expression_mode);
  }
}
   ;

BITXOR
   : '^' [\p{White_Space}]*
   {
  if ((_input->LA(1) == ':' && _input->LA(2) == ':') ||
      (_input->LA(1) != ':')) {
    colon_count = 1;
    more();
    pushMode(expression_mode);
  }
}
   ;

LOGAND
   : '&&' [\p{White_Space}]*
   {
  if ((_input->LA(1) == ':' && _input->LA(2) == ':') ||
      (_input->LA(1) != ':')) {
    colon_count = 1;
    more();
    pushMode(expression_mode);
  }
}
   ;

LOGOR
   : '||' [\p{White_Space}]*
   {
  if ((_input->LA(1) == ':' && _input->LA(2) == ':') ||
      (_input->LA(1) != ':')) {
    colon_count = 1;
    more();
    pushMode(expression_mode);
  }
}
   ;

FORT_AND
   : ('.' [Aa] [Nn] [Dd] '.') [\p{White_Space}]*
   {
  if ((_input->LA(1) == ':' && _input->LA(2) == ':') ||
      (_input->LA(1) != ':')) {
    colon_count = 1;
    more();
    pushMode(expression_mode);
  }
}
   ;

FORT_OR
   : ('.' [Oo] [Rr] '.') [\p{White_Space}]*
   {
  if ((_input->LA(1) == ':' && _input->LA(2) == ':') ||
      (_input->LA(1) != ':')) {
    colon_count = 1;
    more();
    pushMode(expression_mode);
  }
}
   ;

FORT_EQV
   : ('.' [Ee] [Qq] [Vv] '.') [\p{White_Space}]*
   {
  if ((_input->LA(1) == ':' && _input->LA(2) == ':') ||
      (_input->LA(1) != ':')) {
    colon_count = 1;
    more();
    pushMode(expression_mode);
  }
}
   ;

FORT_NEQV
   : ('.' [Nn] [Ee] [Qq] [Vv] '.') [\p{White_Space}]*
   {
  if ((_input->LA(1) == ':' && _input->LA(2) == ':') ||
      (_input->LA(1) != ':')) {
    colon_count = 1;
    more();
    pushMode(expression_mode);
  }
}
   ;

FORT_IAND
   : [Ii] [Aa] [Nn] [Dd] [\p{White_Space}]*
   {
  if ((_input->LA(1) == ':' && _input->LA(2) == ':') ||
      (_input->LA(1) != ':')) {
    colon_count = 1;
    more();
    pushMode(expression_mode);
  }
}
   ;

FORT_IOR
   : [Ii] [Oo] [Rr] [\p{White_Space}]*
   {
  if ((_input->LA(1) == ':' && _input->LA(2) == ':') ||
      (_input->LA(1) != ':')) {
    colon_count = 1;
    more();
    pushMode(expression_mode);
  }
}
   ;

FORT_IEOR
   : [Ii] [Ee] [Oo] [Rr] [\p{White_Space}]*
   {
  if ((_input->LA(1) == ':' && _input->LA(2) == ':') ||
      (_input->LA(1) != ':')) {
    colon_count = 1;
    more();
    pushMode(expression_mode);
  }
}
   ;

REDUCTION_COLON
   : ':' [\p{White_Space}]*
   {
  if (_input->LA(1) == ':')
    more();
  else {
    colon_count = 0;
    setType(COLON);
    pushMode(expression_mode);
  }
}
   ;

REDUCTION_COMMA
   : ',' [\p{White_Space}]*
   {
  skip();
  pushMode(expression_mode);
  parenthesis_global_count = 1;
  parenthesis_local_count = 0;
}
   ;

REDUCTION_BLANK
   : [\p{White_Space}]+ -> skip
   ;

REDUCTION_LINE_END
   : [\n\r] -> skip
   ;

mode wait_clause;
WAIT_LEFT_PAREN
   : '(' [\p{White_Space}]*
   {
  setType(LEFT_PAREN);
  bracket_count = 0;
  parenthesis_global_count = 1;
  parenthesis_local_count = 0;
  if (lookAhead("devnum") == false && lookAhead("queues") == false) {
    colon_count = 0;
    pushMode(expression_mode);
  }
}
   ;

WAIT_RIGHT_PAREN
   : ')' -> type (RIGHT_PAREN) , popMode
   ;

DEVNUM
   : 'devnum' [\p{White_Space}]*
   {
  if ((_input->LA(1) == ':' && _input->LA(2) == ':') ||
      (_input->LA(1) != ':')) {
    more();
    bracket_count = 0;
    colon_count = 1;
    pushMode(expression_mode);
  }
}
   ;

QUEUES
   : 'queues' [\p{White_Space}]*
   {
  if ((_input->LA(1) == ':' && _input->LA(2) == ':') ||
      (_input->LA(1) != ':')) {
    more();
    bracket_count = 0;
    colon_count = 1;
    pushMode(expression_mode);
  }
}
   ;

WAIT_COLON
   : ':' [\p{White_Space}]*
   {
  if (lookAhead("queues") == false) {
    bracket_count = 0;
    colon_count = 0;
    pushMode(expression_mode);
  } else {
    more();
  }
  setType(COLON);
}
   ;

WAIT_COMMA
   : ',' [\p{White_Space}]*
   {
  skip();
  pushMode(expression_mode);
  parenthesis_global_count = 1;
  parenthesis_local_count = 0;
  bracket_count = 0;
  colon_count = 0;
}
   ;

WAIT_BLANK
   : [\p{White_Space}]+ -> skip
   ;

WAIT_LINE_END
   : [\n\r] -> skip
   ;

mode worker_clause;
WORKER_LEFT_PAREN
   : '(' [\p{White_Space}]*
   {
  setType(LEFT_PAREN);
  parenthesis_global_count = 1;
  parenthesis_local_count = 0;
  if (lookAhead("num") == false) {
    pushMode(expression_mode);
  }
}
   ;

WORKER_RIGHT_PAREN
   : ')' -> type (RIGHT_PAREN) , popMode
   ;

WORKER_NUM
   : 'num' [\p{White_Space}]*
   {
  if ((_input->LA(1) == ':' && _input->LA(2) == ':') ||
      (_input->LA(1) != ':')) {
    more();
    pushMode(expression_mode);
  } else if (_input->LA(1) == ':' && _input->LA(2) != ':') {
    setType(NUM);
  }
}
   ;

WORKER_COLON
   : ':' [\p{White_Space}]*
   {
  if (_input->LA(1) == ':')
    more();
  else {
    colon_count = 0;
    setType(COLON);
    pushMode(expression_mode);
  }
}
   ;

WORKER_BLANK
   : [\p{White_Space}]+ -> skip
   ;

WORKER_LINE_END
   : [\n\r] -> skip
   ;

mode bind_clause;
BIND_LEFT_PAREN
   : '(' [\p{White_Space}]* -> type(LEFT_PAREN)
   ;

BIND_RIGHT_PAREN
   : ')' -> type(RIGHT_PAREN) , popMode
   ;

STRING_LITERAL
   : '"' ( '""' | '\\' . | ~["\r\n] )* '"'
   | '\'' ( '\'' '\'' | '\\' . | ~['\r\n] )* '\''
   ;

BIND_NAME
   : ~[ \t\f\r\n()] (~[ \t\f\r\n()])* -> type(EXPR)
   ;

BIND_BLANK
   : [\p{White_Space}]+ -> skip
   ;

BIND_LINE_END
   : [\n\r] -> skip
   ;

mode gang_clause;
GANG_LEFT_PAREN
   : '(' [\p{White_Space}]*
   {
  setType(LEFT_PAREN);
  parenthesis_global_count = 1;
  parenthesis_local_count = 0;
  // If this is a gang clause with key:value args, let the key tokens match.
  // Otherwise, fall back to expression capture (tile/num_gangs or positional args).
  if (gang_clause_allows_keyword_args &&
      (lookAheadKWArgs("num") ||
       lookAheadKWArgs("dim") ||
       lookAhead("static"))) {
    // Do not enter expression_mode yet; consume the key token first.
  } else {
    colon_count = 1;  // Allow colons in gang/tile/num_gangs expressions
    pushMode(expression_mode);
  }
}
   ;

GANG_RIGHT_PAREN
   : ')' -> type (RIGHT_PAREN) , popMode
   ;

GANG_COMMA
   : ',' [\p{White_Space}]*
   {
  skip();
  parenthesis_global_count = 1;
  parenthesis_local_count = 0;
  if (gang_clause_allows_keyword_args &&
      (lookAheadKWArgs("num") ||
       lookAheadKWArgs("dim") ||
       lookAhead("static"))) {
    // Do not enter expression_mode yet; consume the key token first.
  } else {
    colon_count = 1;  // Allow colons in gang/tile/num_gangs expressions
    pushMode(expression_mode);
  }
}
   ;

GANG_NUM
   : 'num' [\p{White_Space}]*
   {
  if (_input->LA(1) != ':') {
    // Not a key:value pair; treat as part of an expression.
    setType(EXPR);
    colon_count = 1;
    pushMode(expression_mode);
    parenthesis_global_count = 1;
    parenthesis_local_count = 0;
    more();
  } else {
    setType(NUM);
  }
}
   ;

DIM
   : 'dim' [\p{White_Space}]*
   {
  if (_input->LA(1) != ':') {
    setType(EXPR);
    colon_count = 1;
    pushMode(expression_mode);
    parenthesis_global_count = 1;
    parenthesis_local_count = 0;
    more();
  }
}
   ;

STATIC
   : 'static'
   ;

GANG_COLON
   : ':' [\p{White_Space}]*
   {
  setType(COLON);
  // After key:value separator, capture the value expression as EXPR.
  colon_count = 1;
  pushMode(expression_mode);
  parenthesis_global_count = 1;
  parenthesis_local_count = 0;
}
   ;

GANG_BLANK
   : [\p{White_Space}]+ -> skip
   ;

GANG_LINE_END
   : [\n\r] -> skip
   ;

mode expr_clause;
EXPR_LEFT_PAREN
   : '(' [\p{White_Space}]*
   {
  setType(LEFT_PAREN);
  pushMode(expression_mode);
  parenthesis_global_count = 1;
  parenthesis_local_count = 0;
}
   ;

EXPR_RIGHT_PAREN
   : ')' -> type (RIGHT_PAREN) , popMode
   ;

COMMA
   : ',' [\p{White_Space}]*
   {
  skip();
  pushMode(expression_mode);
  parenthesis_global_count = 1;
  parenthesis_local_count = 0;
}
   ;

mode expression_mode;
EXPRESSION_LEFT_PAREN
   : '('
   {
  if (paren_processed != true) {
    parenthesis_local_count++;
    parenthesis_global_count++;
  }
  paren_processed = false;
  more();
}
   ;

mode device_type_clause;
DEVICE_TYPE_LEFT_PAREN
   : '(' [\p{White_Space}]* -> type(LEFT_PAREN)
   ;

DEVICE_TYPE_RIGHT_PAREN
   : ')' -> type(RIGHT_PAREN), popMode
   ;

DEVICE_TYPE_COMMA
   : ',' [\p{White_Space}]* -> skip
   ;

DEVICE_TYPE_HOST
   : 'host' -> type(HOST)
   ;

ANY
   : 'any' -> type(ANY)
   ;

DEVICE_TYPE_ANY_STAR
   : '*' -> type(ANY)
   ;

MULTICORE
   : 'multicore' -> type(MULTICORE)
   ;

DEVICE_TYPE_DEFAULT
   : 'default' -> type(DEFAULT)
   ;

DEVICE_TYPE_ID
   : [A-Za-z_] [A-Za-z0-9_]* -> type(EXPR)
   ;

DEVICE_TYPE_BLANK
   : [\p{White_Space}]+ -> skip
   ;

DEVICE_TYPE_LINE_END
   : [\n\r] -> skip
   ;

mode expression_mode;
EXPRESSION_RIGHT_PAREN
   : ')' [\p{White_Space}]*
   {
  if (paren_processed != true) {
    parenthesis_local_count--;
    parenthesis_global_count--;
  }
  paren_processed = false;
  if (parenthesis_global_count == 1 &&
      (lookAhead(")") == true || lookAhead(",") == true)) {
    popMode();
    setType(EXPR);
    parenthesis_local_count = 0;
    parenthesis_global_count = 0;
    bracket_count = 0;
  } else {
    more();
  };
}
   ;

EXPRESSION_CHAR
   : . [\p{White_Space}]*
   {
  switch (_input->LA(1)) {
  case ')': {
    if (parenthesis_global_count - 1 == 0) {
      popMode();
      setType(EXPR);
      parenthesis_local_count = 0;
      bracket_count = 0;
    } else {
      more();
    };
    parenthesis_global_count--;
    parenthesis_local_count--;
    paren_processed = true;
    break;
  }
  case '(': {
    parenthesis_global_count += 1;
    parenthesis_local_count += 1;
    paren_processed = true;
    more();
    break;
  }
  case '[': {
    bracket_count += 1;
    more();
    break;
  }
  case ']': {
    bracket_count -= 1;
    more();
    break;
  }
  case ',': {
    if (parenthesis_local_count == 0) {
      setType(EXPR);
      popMode();
    } else {
      more();
    };
    break;
  }
  case ':': {
    // Only end expression on colon if we're NOT inside any delimiters (parentheses OR brackets)
    if (_input->LA(2) != ':' && colon_count == 0 && bracket_count == 0 && parenthesis_local_count == 0) {
      colon_count = 0;
      setType(EXPR);
      popMode();
    } else {
      // Track colon count for clause modifier separators (e.g., "readonly:")
      // Only track when NOT inside brackets (C array slices [1:10])
      if (bracket_count == 0) {
        // Preserve C++ scope operator '::' regardless of the current colon_count.
        if (_input->LA(2) == ':') {
          colon_count = 1;
        } else if (colon_count == 0) {
          colon_count = 1;
        } else {
          colon_count = 0;
        }
      };
      more();
    };
    break;
  }
  default: {
    more();
  }
  }
}
   ;

