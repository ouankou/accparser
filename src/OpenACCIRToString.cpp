#include "OpenACCIR.h"
#include <cstdarg>

std::string OpenACCDirective::generatePragmaString(std::string prefix,
                                                   std::string beginning_symbol,
                                                   std::string ending_symbol) {

  if (this->getBaseLang() == ACC_Lang_Fortran && prefix == "#pragma acc ") {
    prefix = "!$acc ";
  };
  std::string result = prefix;

  result += this->toString();

  result += beginning_symbol;

  switch (this->getKind()) {
  case ACCD_end: {
    result += ((OpenACCEndDirective *)this)
                  ->getPairedDirective()
                  ->generatePragmaString("", "", "");
    break;
  }
  default: {
    ;
  }
  };

  std::vector<OpenACCClause *> *clauses = this->getClausesInOriginalOrder();
  if (clauses->size() != 0) {
    bool first = true;
    for (auto *clause : *clauses) {
      std::string clause_str = clause->toString();
      // Strip trailing whitespace the clause printer may have added.
      while (!clause_str.empty() && std::isspace(clause_str.back())) {
        clause_str.pop_back();
      }
      if (clause_str.empty()) {
        continue;
      }
      if (!first) {
        result += " ";
      }
      result += clause_str;
      first = false;
    }
  }
  result += ending_symbol;

  return result;
};

std::string OpenACCDirective::toString() {

  std::string result;

  switch (this->getKind()) {
  case ACCD_atomic:
    result += "atomic ";
    break;
  case ACCD_data:
    result += "data ";
    break;
  case ACCD_declare:
    result += "declare ";
    break;
  case ACCD_end:
    result += "end ";
    break;
  case ACCD_enter_data:
    result += "enter data ";
    break;
  case ACCD_exit_data:
    result += "exit data ";
    break;
  case ACCD_host_data:
    result += "host_data ";
    break;
  case ACCD_init:
    result += "init ";
    break;
  case ACCD_kernels:
    result += "kernels ";
    break;
  case ACCD_kernels_loop:
    result += "kernels loop ";
    break;
  case ACCD_loop:
    result += "loop ";
    break;
  case ACCD_parallel:
    result += "parallel ";
    break;
  case ACCD_parallel_loop:
    result += "parallel loop ";
    break;
  case ACCD_routine:
    result += "routine ";
    if (!((OpenACCRoutineDirective *)this)->getName().empty()) {
      result += "(" + ((OpenACCRoutineDirective *)this)->getName() + ") ";
    }
    break;
  case ACCD_serial:
    result += "serial ";
    break;
  case ACCD_serial_loop:
    result += "serial loop ";
    break;
  case ACCD_set:
    result += "set ";
    break;
  case ACCD_shutdown:
    result += "shutdown ";
    break;
  case ACCD_update:
    result += "update ";
    break;
  default:
    printf("The directive enum is not supported yet.\n");
    assert(0);
  };

  return result;
};

std::string OpenACCClause::expressionToString() {

  std::string result;
  std::vector<std::string> *expr = this->getExpressions();
  if (expr != NULL && !expr->empty()) {
    for (auto it = expr->begin(); it != expr->end(); ++it) {
      if (it != expr->begin()) {
        result += ", ";
      }
      result += *it;
    }
  }

  return result;
};

std::string OpenACCClause::toString() {

  std::string result;

  switch (this->getKind()) {
  case ACCC_async:
    return static_cast<OpenACCAsyncClause *>(this)->OpenACCAsyncClause::toString();
  case ACCC_attach:
    return static_cast<OpenACCAttachClause *>(this)
        ->OpenACCAttachClause::toString();
  case ACCC_auto:
    result += "auto ";
    break;
  case ACCC_bind:
    return static_cast<OpenACCBindClause *>(this)->OpenACCBindClause::toString();
  case ACCC_capture:
    result += "capture ";
    break;
  case ACCC_delete:
    return static_cast<OpenACCDeleteClause *>(this)
        ->OpenACCDeleteClause::toString();
  case ACCC_detach:
    return static_cast<OpenACCDetachClause *>(this)
        ->OpenACCDetachClause::toString();
  case ACCC_collapse:
    return static_cast<OpenACCCollapseClause *>(this)
        ->OpenACCCollapseClause::toString();
  case ACCC_copy:
    return static_cast<OpenACCCopyClause *>(this)->OpenACCCopyClause::toString();
  case ACCC_copyin:
    return static_cast<OpenACCCopyinClause *>(this)
        ->OpenACCCopyinClause::toString();
  case ACCC_copyout:
    return static_cast<OpenACCCopyoutClause *>(this)
        ->OpenACCCopyoutClause::toString();
  case ACCC_create:
    return static_cast<OpenACCCreateClause *>(this)
        ->OpenACCCreateClause::toString();
  case ACCC_default_async:
    return static_cast<OpenACCDefaultAsyncClause *>(this)
        ->OpenACCDefaultAsyncClause::toString();
  case ACCC_device:
    return static_cast<OpenACCDeviceClause *>(this)->OpenACCDeviceClause::toString();
  case ACCC_device_num:
    return static_cast<OpenACCDeviceNumClause *>(this)
        ->OpenACCDeviceNumClause::toString();
  case ACCC_device_resident:
    return static_cast<OpenACCDeviceResidentClause *>(this)
        ->OpenACCDeviceResidentClause::toString();
  case ACCC_device_type:
    return static_cast<OpenACCDeviceTypeClause *>(this)
        ->OpenACCDeviceTypeClause::toString();
  case ACCC_deviceptr:
    return static_cast<OpenACCDeviceptrClause *>(this)
        ->OpenACCDeviceptrClause::toString();
  case ACCC_finalize:
    result += "finalize ";
    break;
  case ACCC_firstprivate:
    return static_cast<OpenACCFirstprivateClause *>(this)
        ->OpenACCFirstprivateClause::toString();
  case ACCC_gang:
    return static_cast<OpenACCGangClause *>(this)->OpenACCGangClause::toString();
  case ACCC_host:
    return static_cast<OpenACCHostClause *>(this)->OpenACCHostClause::toString();
  case ACCC_if:
    return static_cast<OpenACCIfClause *>(this)->OpenACCIfClause::toString();
  case ACCC_if_present:
    result += "if_present ";
    break;
  case ACCC_independent:
    result += "independent ";
    break;
  case ACCC_link:
    return static_cast<OpenACCLinkClause *>(this)->OpenACCLinkClause::toString();
  case ACCC_nohost:
    result += "nohost ";
    break;
  case ACCC_no_create:
    return static_cast<OpenACCNoCreateClause *>(this)
        ->OpenACCNoCreateClause::toString();
  case ACCC_num_gangs:
    return static_cast<OpenACCNumGangsClause *>(this)
        ->OpenACCNumGangsClause::toString();
  case ACCC_num_workers:
    return static_cast<OpenACCNumWorkersClause *>(this)
        ->OpenACCNumWorkersClause::toString();
  case ACCC_present:
    return static_cast<OpenACCPresentClause *>(this)
        ->OpenACCPresentClause::toString();
  case ACCC_private:
    return static_cast<OpenACCPrivateClause *>(this)
        ->OpenACCPrivateClause::toString();
  case ACCC_read:
    result += "read ";
    break;
  case ACCC_self:
    return static_cast<OpenACCSelfClause *>(this)->OpenACCSelfClause::toString();
  case ACCC_seq:
    result += "seq ";
    break;
  case ACCC_tile:
    return static_cast<OpenACCTileClause *>(this)->OpenACCTileClause::toString();
  case ACCC_update:
    result += "update ";
    break;
  case ACCC_use_device:
    return static_cast<OpenACCUseDeviceClause *>(this)
        ->OpenACCUseDeviceClause::toString();
  case ACCC_vector_length:
    return static_cast<OpenACCVectorLengthClause *>(this)
        ->OpenACCVectorLengthClause::toString();
  case ACCC_wait:
    result += "wait ";
    break;
  case ACCC_write:
    result += "write ";
    break;
  case ACCC_vector:
    return static_cast<OpenACCVectorClause *>(this)->OpenACCVectorClause::toString();
  case ACCC_worker:
    return static_cast<OpenACCWorkerClause *>(this)->OpenACCWorkerClause::toString();
  default:
    std::cerr << "Unsupported OpenACC clause kind in toString(): "
              << this->getKind() << std::endl;
    assert(false && "Unsupported OpenACC clause kind in toString()");
  }

  std::string clause_string = "(";
  clause_string += this->expressionToString();
  clause_string += ") ";
  if (clause_string.size() > 3) {
    // Remove trailing space from clause name before adding parenthesized expression
    if (!result.empty() && result.back() == ' ') {
      result.pop_back();
    }
    result += clause_string;
  }

  return result;
};

static std::string deviceTypeToString(OpenACCDeviceTypeKind kind) {
  switch (kind) {
  case ACCC_DEVICE_TYPE_host:
    return "host";
  case ACCC_DEVICE_TYPE_any:
    return "any";
  case ACCC_DEVICE_TYPE_multicore:
    return "multicore";
  case ACCC_DEVICE_TYPE_default:
    return "default";
  default:
    return "";
  }
}

std::string OpenACCDeviceTypeClause::toString() {
  std::string result = "device_type";
  std::string clause_string = "";

  bool first = true;
  for (auto kind : device_types) {
    std::string name = deviceTypeToString(kind);
    if (name.empty()) {
      continue;
    }
    if (!first) {
      clause_string += ", ";
    }
    clause_string += name;
    first = false;
  }
  for (const auto &raw : unknown_types) {
    if (!first) {
      clause_string += ", ";
    }
    clause_string += raw;
    first = false;
  }

  if (!clause_string.empty()) {
    result += "(" + clause_string + ") ";
  } else {
    result += " ";
  }
  return result;
}

std::string OpenACCCacheDirective::toString() {

  std::string result = "cache";
  std::string parameter_string = "";
  OpenACCCacheDirectiveModifier modifier = this->getModifier();
  switch (modifier) {
  case ACCC_CACHE_readonly:
    parameter_string = "readonly: ";
    break;
  default:;
  };
  parameter_string += this->varsToString();
  if (!parameter_string.empty()) {
    result += "(" + parameter_string + ") ";
    return result;
  }

  // No vars: drop the trailing space when no modifier or vars were present.
  result += " ";

  return result;
};

std::string OpenACCCollapseClause::toString() {

  std::string result;
  const auto &vals = getCounts();
  if (vals.empty()) {
    result = "collapse ";
    return result;
  }

  for (const auto &val : vals) {
    result += "collapse";
    result += "(" + val + ") ";
  }
  return result;
}

std::string OpenACCAsyncClause::toString() {

  std::string result = "async";
  if (getModifier() == ACCC_ASYNC_expr && !getAsyncExpr().empty()) {
    result += "(" + getAsyncExpr() + ") ";
  } else {
    result += " ";
  }
  return result;
}

std::string OpenACCWaitDirective::expressionToString() {

  std::string result;
  std::vector<std::string> *expr = this->getAsyncIds();
  if (expr != NULL && !expr->empty()) {
    for (auto it = expr->begin(); it != expr->end(); ++it) {
      if (it != expr->begin()) {
        result += ", ";
      }
      result += *it;
    }
  }

  return result;
};

std::string OpenACCWaitDirective::toString() {

  std::string result = "wait";
  std::string parameter_string = "";
  if (!this->getAsyncIds()->empty()) {
    result += "(";
    std::string devnum = this->getDevnum();
    if (devnum != "") {
      parameter_string += "devnum: " + devnum + ": ";
    };
    if (this->getQueues() == true) {
      parameter_string += "queues: ";
    };

    parameter_string += this->expressionToString();
    result += parameter_string + ") ";
  } else {
    result += " ";
  }
  return result;
};

std::string OpenACCDefaultAsyncClause::toString() {

  std::string result = "default_async";
  if (!getAsyncExpr().empty()) {
    result += "(" + getAsyncExpr() + ") ";
  } else {
    result += " ";
  }
  return result;
}

std::string OpenACCDeviceClause::toString() {

  std::string result = "device";
  const auto &devs = getDevices();
  if (!devs.empty()) {
    result += "(";
    for (auto it = devs.begin(); it != devs.end(); ++it) {
      result += *it;
      if (it + 1 != devs.end()) {
        result += ", ";
      }
    }
    result += ") ";
  } else {
    result += " ";
  }
  return result;
}

std::string OpenACCIfClause::toString() {

  std::string result = "if";
  if (!getCondition().empty()) {
    result += "(" + getCondition() + ") ";
  } else {
    result += " ";
  }
  return result;
}

std::string OpenACCDeviceNumClause::toString() {

  std::string result = "device_num";
  if (!getDeviceExpr().empty()) {
    result += "(" + getDeviceExpr() + ") ";
  } else {
    result += " ";
  }
  return result;
}

std::string OpenACCGangClause::toString() {

  std::string result = "gang";
  if (getArgs().empty()) {
    result += " ";
    return result;
  }

  std::string parameter_string;
  const auto &arg_list = getArgs();
  for (auto it = arg_list.begin(); it != arg_list.end(); ++it) {
    if (it != arg_list.begin()) {
      parameter_string += ", ";
    }
    switch (it->kind) {
    case ACCC_GANG_ARG_num:
      parameter_string += "num:" + it->value;
      break;
    case ACCC_GANG_ARG_dim:
      parameter_string += "dim:" + it->value;
      break;
    case ACCC_GANG_ARG_static:
      parameter_string += "static:" + it->value;
      break;
    default:
      parameter_string += it->value;
      break;
    }
  }
  result += "(" + parameter_string + ") ";
  return result;
}

std::string OpenACCNumGangsClause::toString() {

  std::string result = "num_gangs";
  const auto &vals = getNums();
  if (!vals.empty()) {
    result += "(";
    for (auto it = vals.begin(); it != vals.end(); ++it) {
      result += *it;
      if (it + 1 != vals.end())
        result += ", ";
    }
    result += ") ";
  } else {
    result += " ";
  }
  return result;
}

std::string OpenACCNumWorkersClause::toString() {

  std::string result = "num_workers";
  if (!getNumExpr().empty()) {
    result += "(" + getNumExpr() + ") ";
  } else {
    result += " ";
  }
  return result;
}

std::string OpenACCTileClause::toString() {

  std::string result = "tile";
  const auto &sizes = getTileSizes();
  if (!sizes.empty()) {
    result += "(";
    for (auto it = sizes.begin(); it != sizes.end(); ++it) {
      result += *it;
      if (it + 1 != sizes.end()) {
        result += ", ";
      }
    }
    result += ") ";
  } else {
    result += " ";
  }
  return result;
}

std::string OpenACCBindClause::toString() {

  std::string result = "bind";
  if (!getBinding().empty()) {
    result += "(" + getBinding() + ") ";
  } else {
    result += " ";
  }
  return result;
}

std::string OpenACCCopyinClause::toString() {

  std::string keyword = original_keyword.empty() ? "copyin" : original_keyword;
  std::string result = keyword;
  std::string parameter_string = "";
  OpenACCCopyinClauseModifier modifier = this->getModifier();
  switch (modifier) {
  case ACCC_COPYIN_readonly:
    parameter_string = "readonly: ";
    break;
  default:;
  };
  parameter_string += this->varsToString();
  if (!parameter_string.empty()) {
    result += "(" + parameter_string + ") ";
  } else {
    result += " ";
  }

  return result;
};

std::string OpenACCCopyoutClause::toString() {

  std::string keyword = original_keyword.empty() ? "copyout" : original_keyword;
  std::string result = keyword;
  std::string parameter_string = "";
  OpenACCCopyoutClauseModifier modifier = this->getModifier();
  switch (modifier) {
  case ACCC_COPYOUT_zero:
    parameter_string = "zero: ";
    break;
  default:;
  };
  parameter_string += this->varsToString();
  if (!parameter_string.empty()) {
    result += "(" + parameter_string + ") ";
  } else {
    result += " ";
  }

  return result;
};

std::string OpenACCCreateClause::toString() {

  std::string keyword = original_keyword.empty() ? "create" : original_keyword;
  std::string result = keyword;
  std::string parameter_string = "";
  OpenACCCreateClauseModifier modifier = this->getModifier();
  switch (modifier) {
  case ACCC_CREATE_zero:
    parameter_string = "zero: ";
    break;
  default:;
  };
  parameter_string += this->varsToString();
  if (!parameter_string.empty()) {
    result += "(" + parameter_string + ") ";
  } else {
    result += " ";
  }

  return result;
};

static std::string
varClauseToString(const std::string &keyword,
                  const std::vector<std::string> &vars) {
  std::string result = keyword;
  if (!vars.empty()) {
    result += "(";
    for (auto it = vars.begin(); it != vars.end(); ++it) {
      result += *it;
      if (it + 1 != vars.end()) {
        result += ", ";
      }
    }
    result += ") ";
  } else {
    result += " ";
  }
  return result;
}

std::string OpenACCCopyClause::toString() {
  std::string keyword = original_keyword.empty() ? "copy" : original_keyword;
  std::string result = keyword;
  if (!getVars().empty()) {
    result += "(" + varsToString() + ") ";
  } else {
    result += " ";
  }
  return result;
}

std::string OpenACCNoCreateClause::toString() {
  std::string result = "no_create";
  if (!getVars().empty()) {
    result += "(" + varsToString() + ") ";
  } else {
    result += " ";
  }
  return result;
}

std::string OpenACCPresentClause::toString() {
  std::string result = "present";
  if (!getVars().empty()) {
    result += "(" + varsToString() + ") ";
  } else {
    result += " ";
  }
  return result;
}

std::string OpenACCLinkClause::toString() {
  std::string result = "link";
  if (!getVars().empty()) {
    result += "(" + varsToString() + ") ";
  } else {
    result += " ";
  }
  return result;
}

std::string OpenACCDeviceResidentClause::toString() {
  std::string result = "device_resident";
  if (!getVars().empty()) {
    result += "(" + varsToString() + ") ";
  } else {
    result += " ";
  }
  return result;
}

std::string OpenACCDeviceptrClause::toString() {
  std::string result = "deviceptr";
  if (!getVars().empty()) {
    result += "(" + varsToString() + ") ";
  } else {
    result += " ";
  }
  return result;
}

std::string OpenACCUseDeviceClause::toString() {
  std::string result = "use_device";
  if (!getVars().empty()) {
    result += "(" + varsToString() + ") ";
  } else {
    result += " ";
  }
  return result;
}

std::string OpenACCAttachClause::toString() {
  std::string result = "attach";
  if (!getVars().empty()) {
    result += "(" + varsToString() + ") ";
  } else {
    result += " ";
  }
  return result;
}

std::string OpenACCDeleteClause::toString() {
  return varClauseToString("delete", getVars());
}

std::string OpenACCDetachClause::toString() {
  return varClauseToString("detach", getVars());
}

std::string OpenACCSelfClause::toString() {
  std::string result = "self";
  if (!getCondition().empty()) {
    result += "(" + getCondition() + ")";
  } else if (!getVars().empty()) {
    result += "(" + varsToString() + ")";
  }
  result += " ";
  return result;
}

std::string OpenACCFirstprivateClause::toString() {
  return varClauseToString("firstprivate", getVars());
}

std::string OpenACCHostClause::toString() {
  return varClauseToString("host", getVars());
}

std::string OpenACCPrivateClause::toString() {
  return varClauseToString("private", getVars());
}

std::string OpenACCDefaultClause::toString() {

  std::string result = "default";
  std::string parameter_string;
  OpenACCDefaultClauseKind default_kind = this->getKind();
  switch (default_kind) {
  case ACCC_DEFAULT_present:
    parameter_string = "present";
    break;
  case ACCC_DEFAULT_none:
    parameter_string = "none";
    break;
  default:
    std::cout << "The parameter of default clause is not supported.\n";
  };

  if (parameter_string.size() > 0) {
    result += "(" + parameter_string + ") ";
  } else {
    result += " ";
  }

  return result;
};

std::string OpenACCReductionClause::toString() {

  std::string result = "reduction(";
  std::string op;
  OpenACCReductionClauseOperator reduction_operator = this->getOperator();
  switch (reduction_operator) {
  case ACCC_REDUCTION_add:
    op = "+";
    break;
  case ACCC_REDUCTION_sub:
    op = "-";
    break;
  case ACCC_REDUCTION_mul:
    op = "*";
    break;
  case ACCC_REDUCTION_max:
    op = "max";
    break;
  case ACCC_REDUCTION_min:
    op = "min";
    break;
  case ACCC_REDUCTION_bitand:
    op = "&";
    break;
  case ACCC_REDUCTION_bitor:
    op = "|";
    break;
  case ACCC_REDUCTION_bitxor:
    op = "^";
    break;
  case ACCC_REDUCTION_logand:
    op = "&&";
    break;
  case ACCC_REDUCTION_logor:
    op = "||";
    break;
  case ACCC_REDUCTION_fort_and:
    op = ".and.";
    break;
  case ACCC_REDUCTION_fort_or:
    op = ".or.";
    break;
  case ACCC_REDUCTION_fort_eqv:
    op = ".eqv.";
    break;
  case ACCC_REDUCTION_fort_neqv:
    op = ".neqv.";
    break;
  case ACCC_REDUCTION_fort_iand:
    op = ".iand.";
    break;
  case ACCC_REDUCTION_fort_ior:
    op = ".ior.";
    break;
  case ACCC_REDUCTION_fort_ieor:
    op = ".ieor.";
    break;
  default:
    op = "";
  };

  result += op;
  result += " : ";
  result += this->varsToString();
  result += ") ";

  return result;
};

std::string OpenACCVectorClause::toString() {

  std::string result = "vector";
  const std::string &length = this->getLengthExpr();
  switch (this->getModifier()) {
  case ACCC_VECTOR_length:
    result += "(length: " + length + ") ";
    break;
  case ACCC_VECTOR_expr_only:
    result += "(" + length + ") ";
    break;
  default:
    if (!length.empty()) {
      result += "(" + length + ") ";
    } else {
      result += " ";
    }
  }

  return result;
};

std::string OpenACCVectorLengthClause::toString() {

  std::string result = "vector_length";
  const std::string &length = this->getLengthExpr();
  if (!length.empty()) {
    result += "(" + length + ") ";
  } else {
    result += " ";
  }
  return result;
}

std::string OpenACCWaitClause::toString() {

  std::string result = "wait";
  std::string parameter_string = "";
  if (!this->getAsyncIds().empty()) {
    result += "(";
    std::string devnum = this->getDevnum();
    if (devnum != "") {
      parameter_string += "devnum: " + devnum + ": ";
    };
    if (this->getQueues() == true) {
      parameter_string += "queues: ";
    };

    const auto &ids = this->getAsyncIds();
    for (auto it = ids.begin(); it != ids.end(); ++it) {
      parameter_string += *it;
      if (it + 1 != ids.end()) {
        parameter_string += ", ";
      }
    }
    result += parameter_string + ") ";
  } else {
    result += " ";
  }

  return result;
};

std::string OpenACCWorkerClause::toString() {

  std::string result = "worker";
  const std::string &num = this->getNumExpr();
  switch (this->getModifier()) {
  case ACCC_WORKER_num:
    result += "(num: " + num + ") ";
    break;
  case ACCC_WORKER_expr_only:
    result += "(" + num + ") ";
    break;
  default:
    if (!num.empty()) {
      result += "(" + num + ") ";
    } else {
      result += " ";
    }
  }

  return result;
};
