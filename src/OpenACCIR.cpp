#include "OpenACCIR.h"
#include <algorithm>
#include <cstdarg>

// Initialize static flag - disable merging by default to preserve clause
// occurrences and ordering.
bool OpenACCDirective::enable_clause_merging = false;

void OpenACCClause::addLangExpr(const OpenACCExpressionItem &expression,
                                int line, int col) {
  if (isClauseMergingEnabled()) {
    for (const auto &prev : expressions) {
      if (prev.text == expression.text) {
        return;
      }
    }
  }
  expressions.push_back(expression);
  locations.push_back(ACC_SourceLocation(line, col));
};

/**
 *
 * @param kind
 * @param ..., parameters for the clause, the number of max number of parameters
 * is determined by the kind since each clause expects a fixed set of
 * parameters.
 * @return
 */
OpenACCClause *OpenACCDirective::addOpenACCClause(int k, ...) {

  OpenACCClauseKind kind = (OpenACCClauseKind)k;
  // Check whether the given kind of clause exists first.
  // If not, create an empty vector.
  if (clauses.count(kind) == 0) {
    clauses.insert(std::pair<OpenACCClauseKind, std::vector<OpenACCClause *> *>(
        kind, new std::vector<OpenACCClause *>));
  };
  std::vector<OpenACCClause *> *current_clauses = getClauses(kind);
  va_list args;
  va_start(args, k);
  OpenACCClause *new_clause = NULL;

  switch (kind) {
  case ACCC_auto:
  case ACCC_capture:
  case ACCC_finalize:
  case ACCC_if_present:
  case ACCC_independent:
  case ACCC_nohost:
  case ACCC_read:
  case ACCC_seq:
  case ACCC_update:
  case ACCC_write: {
    if (current_clauses->size() == 0) {
      new_clause = new OpenACCClause(kind);
      current_clauses = new std::vector<OpenACCClause *>();
      current_clauses->push_back(new_clause);
      clauses[kind] = current_clauses;
    } else {
      if (kind == ACCC_if) {
        std::cerr << "Cannot have two if clauses for the directive " << kind
                  << ", ignored\n";
      } else if (enable_clause_merging) {
        /* we can have multiple clause and we merge them together now, thus we
         * return the object that is already created */
        new_clause = current_clauses->at(0);
      } else {
        /* merging is disabled - create a new clause instance */
        new_clause = new OpenACCClause(kind);
        current_clauses->push_back(new_clause);
      }
    }
    break;
  }
  case ACCC_device_type: {
    if (current_clauses->size() == 0 || !enable_clause_merging) {
      new_clause = OpenACCDeviceTypeClause::addClause(this);
      if (current_clauses->size() == 0) {
        current_clauses = new std::vector<OpenACCClause *>();
        clauses[kind] = current_clauses;
      }
      current_clauses->push_back(new_clause);
    } else {
      new_clause = current_clauses->at(0);
    }
    break;
  }
  case ACCC_gang: {
    if (this->getKind() == ACCD_routine) {
      if (current_clauses->size() == 0) {
        new_clause = new OpenACCGangClause();
        current_clauses = new std::vector<OpenACCClause *>();
        current_clauses->push_back(new_clause);
        clauses[kind] = current_clauses;
      } else {
        /* we can have multiple clause and we merge them together now, thus we
         * return the object that is already created */
        new_clause = current_clauses->at(0);
      }
      break;
    } else {
      if (current_clauses->size() == 0) {
        new_clause = new OpenACCGangClause();
        current_clauses = new std::vector<OpenACCClause *>();
        current_clauses->push_back(new_clause);
        clauses[kind] = current_clauses;
      } else {
        new_clause = new OpenACCGangClause();
        current_clauses->push_back(new_clause);
      }
      break;
    }
  }

  case ACCC_self: {
    new_clause = OpenACCSelfClause::addClause(this);
    break;
  }
  case ACCC_async:
    new_clause = OpenACCAsyncClause::addClause(this);
    break;
  case ACCC_delete: {
    new_clause = OpenACCDeleteClause::addClause(this);
    break;
  }
  case ACCC_detach: {
    new_clause = OpenACCDetachClause::addClause(this);
    break;
  }
  case ACCC_collapse: {
    new_clause = OpenACCCollapseClause::addClause(this);
    break;
  }
  case ACCC_attach: {
    new_clause = OpenACCAttachClause::addClause(this);
    break;
  }
  case ACCC_copy: {
    new_clause = OpenACCCopyClause::addClause(this);
    break;
  }
  case ACCC_device: {
    new_clause = OpenACCDeviceClause::addClause(this);
    break;
  }
  case ACCC_device_resident: {
    new_clause = OpenACCDeviceResidentClause::addClause(this);
    break;
  }
  case ACCC_deviceptr: {
    new_clause = OpenACCDeviceptrClause::addClause(this);
    break;
  }
  case ACCC_default_async: {
    new_clause = OpenACCDefaultAsyncClause::addClause(this);
    break;
  }
  case ACCC_if: {
    new_clause = OpenACCIfClause::addClause(this);
    break;
  }
  case ACCC_firstprivate: {
    new_clause = OpenACCFirstprivateClause::addClause(this);
    break;
  }
  case ACCC_num_workers: {
    new_clause = OpenACCNumWorkersClause::addClause(this);
    break;
  }
  case ACCC_no_create: {
    new_clause = OpenACCNoCreateClause::addClause(this);
    break;
  }
  case ACCC_device_num: {
    new_clause = OpenACCDeviceNumClause::addClause(this);
    break;
  }
  case ACCC_present: {
    new_clause = OpenACCPresentClause::addClause(this);
    break;
  }
  case ACCC_private: {
    new_clause = OpenACCPrivateClause::addClause(this);
    break;
  }
  case ACCC_num_gangs: {
    new_clause = OpenACCNumGangsClause::addClause(this);
    break;
  }
  case ACCC_link: {
    new_clause = OpenACCLinkClause::addClause(this);
    break;
  }
  case ACCC_host: {
    new_clause = OpenACCHostClause::addClause(this);
    break;
  }
  case ACCC_tile: {
    new_clause = OpenACCTileClause::addClause(this);
    break;
  }
  case ACCC_bind: {
    new_clause = OpenACCBindClause::addClause(this);
    break;
  }
  case ACCC_copyin: {
    new_clause = OpenACCCopyinClause::addClause(this);
    break;
  }
  case ACCC_copyout: {
    new_clause = OpenACCCopyoutClause::addClause(this);
    break;
  }
  case ACCC_create: {
    new_clause = OpenACCCreateClause::addClause(this);
    break;
  }
  case ACCC_default: {
    new_clause = OpenACCDefaultClause::addClause(this);
    break;
  }
  case ACCC_reduction: {
    new_clause = OpenACCReductionClause::addClause(this);
    break;
  }
  case ACCC_use_device: {
    new_clause = OpenACCUseDeviceClause::addClause(this);
    break;
  }
  case ACCC_vector: {
    new_clause = OpenACCVectorClause::addClause(this);
    break;
  }
  case ACCC_vector_length: {
    new_clause = OpenACCVectorLengthClause::addClause(this);
    break;
  }
  case ACCC_wait: {
    new_clause = OpenACCWaitClause::addClause(this);
    break;
  }
  case ACCC_worker: {
    new_clause = OpenACCWorkerClause::addClause(this);
    break;
  }
  default: {
    std::cout << "Unknown OpenACC clause!\n";
    assert(0);
  }
  };

  va_end(args);
  if (new_clause != NULL && new_clause->getClausePosition() == -1) {
    this->getClausesInOriginalOrder()->push_back(new_clause);
    new_clause->setClausePosition(this->getClausesInOriginalOrder()->size() -
                                  1);
  };
  return new_clause;
};

void OpenACCAsyncClause::mergeClause(OpenACCDirective *directive,
                                     OpenACCClause *current_clause) {
  // Respect the global clause merging flag
  if (!OpenACCDirective::getClauseMerging()) {
    return;
  }

  std::vector<OpenACCClause *> *current_clauses =
      directive->getClauses(ACCC_async);

  if (current_clauses->size() < 2) {
    return;
  }

  auto *incoming = static_cast<OpenACCAsyncClause *>(current_clause);
  for (auto it = current_clauses->begin(); it != current_clauses->end() - 1;
       ++it) {
    auto *existing = static_cast<OpenACCAsyncClause *>(*it);
    if (incoming->getModifier() == existing->getModifier() &&
        incoming->getAsyncExpr().text == existing->getAsyncExpr().text) {
      current_clauses->pop_back();
      directive->getClausesInOriginalOrder()->pop_back();
      delete incoming;
      break;
    }
  }
};

OpenACCClause *OpenACCAsyncClause::addClause(OpenACCDirective *directive) {

  std::map<OpenACCClauseKind, std::vector<OpenACCClause *> *> *all_clauses =
      directive->getAllClauses();
  std::vector<OpenACCClause *> *current_clauses =
      directive->getClauses(ACCC_async);
  OpenACCClause *new_clause = NULL;
  if (current_clauses->size() == 0) {
    new_clause = new OpenACCAsyncClause();
    current_clauses = new std::vector<OpenACCClause *>();
    current_clauses->push_back(new_clause);
    (*all_clauses)[ACCC_async] = current_clauses;
  } else {
    new_clause = new OpenACCAsyncClause();
    current_clauses->push_back(new_clause);
  }
  return new_clause;
}

void OpenACCBindClause::mergeClause(OpenACCDirective *directive,
                                    OpenACCClause *current_clause) {
  // Respect the global clause merging flag
  if (!OpenACCDirective::getClauseMerging()) {
    return;
  }

  std::vector<OpenACCClause *> *current_clauses =
      directive->getClauses(ACCC_bind);

  if (current_clauses->size() < 2) {
    return;
  }

  auto *incoming = static_cast<OpenACCBindClause *>(current_clause);
  for (auto it = current_clauses->begin(); it != current_clauses->end() - 1;
       ++it) {
    auto *existing = static_cast<OpenACCBindClause *>(*it);
    if (existing->getBinding().text == incoming->getBinding().text &&
        existing->isStringLiteral() == incoming->isStringLiteral()) {
      current_clauses->pop_back();
      directive->getClausesInOriginalOrder()->pop_back();
      delete incoming;
      break;
    }
  }
};

OpenACCClause *OpenACCBindClause::addClause(OpenACCDirective *directive) {

  std::map<OpenACCClauseKind, std::vector<OpenACCClause *> *> *all_clauses =
      directive->getAllClauses();
  std::vector<OpenACCClause *> *current_clauses =
      directive->getClauses(ACCC_bind);
  OpenACCClause *new_clause = NULL;
  if (current_clauses->size() == 0) {
    new_clause = new OpenACCBindClause();
    current_clauses = new std::vector<OpenACCClause *>();
    current_clauses->push_back(new_clause);
    (*all_clauses)[ACCC_bind] = current_clauses;
  } else {
    new_clause = new OpenACCBindClause();
    current_clauses->push_back(new_clause);
  }
  return new_clause;
}

void OpenACCCollapseClause::mergeClause(OpenACCDirective *directive,
                                        OpenACCClause *current_clause) {
  // Respect the global clause merging flag
  if (!OpenACCDirective::getClauseMerging()) {
    return;
  }

  auto *current_clauses = directive->getClauses(ACCC_collapse);
  if (current_clauses->size() < 2) {
    return;
  }

  auto *incoming = static_cast<OpenACCCollapseClause *>(current_clause);
  for (auto it = current_clauses->begin(); it != current_clauses->end() - 1;
       ++it) {
    auto *existing = static_cast<OpenACCCollapseClause *>(*it);

    bool merged = false;
    if (existing->getCounts().empty() && incoming->getCounts().empty()) {
      merged = true;
    } else if (!incoming->getCounts().empty()) {
      merged = true;
      for (const auto &count : incoming->getCounts()) {
        bool found = false;
        for (const auto &prev : existing->getCounts()) {
          if (prev.text == count.text) {
            found = true;
            break;
          }
        }
        if (!found) {
          existing->addCountExpr(count);
        }
      }
    }

    if (merged) {
      current_clauses->pop_back();
      directive->getClausesInOriginalOrder()->pop_back();
      delete incoming;
      break;
    }
  }
};

static void mergeVarList(OpenACCVarListClause *existing,
                         OpenACCVarListClause *incoming) {
  for (const auto &var : incoming->getVars()) {
    bool seen = false;
    for (const auto &prev : existing->getVars()) {
      if (prev.text == var.text) {
        seen = true;
        break;
      }
    }
    if (!seen) {
      existing->addVar(var);
    }
  }
}

void OpenACCAttachClause::mergeClause(OpenACCDirective *directive,
                                      OpenACCClause *current_clause) {
  if (!OpenACCDirective::getClauseMerging()) {
    return;
  }
  auto *current_clauses = directive->getClauses(ACCC_attach);
  if (current_clauses->size() < 2) {
    return;
  }
  auto *incoming = static_cast<OpenACCAttachClause *>(current_clause);
  auto *existing =
      static_cast<OpenACCAttachClause *>(current_clauses->front());
  mergeVarList(existing, incoming);
  current_clauses->pop_back();
  directive->getClausesInOriginalOrder()->pop_back();
  delete incoming;
}

OpenACCClause *OpenACCAttachClause::addClause(OpenACCDirective *directive) {
  auto *all_clauses = directive->getAllClauses();
  auto *current_clauses = directive->getClauses(ACCC_attach);
  OpenACCClause *new_clause = nullptr;
  if (current_clauses->size() == 0) {
    new_clause = new OpenACCAttachClause();
    current_clauses = new std::vector<OpenACCClause *>();
    current_clauses->push_back(new_clause);
    (*all_clauses)[ACCC_attach] = current_clauses;
  } else if (OpenACCDirective::getClauseMerging()) {
    new_clause = current_clauses->at(0);
  } else {
    new_clause = new OpenACCAttachClause();
    current_clauses->push_back(new_clause);
  }
  return new_clause;
}

void OpenACCDeleteClause::mergeClause(OpenACCDirective *directive,
                                      OpenACCClause *current_clause) {
  if (!OpenACCDirective::getClauseMerging()) {
    return;
  }
  auto *current_clauses = directive->getClauses(ACCC_delete);
  if (current_clauses->size() < 2) {
    return;
  }
  auto *incoming = static_cast<OpenACCDeleteClause *>(current_clause);
  auto *existing =
      static_cast<OpenACCDeleteClause *>(current_clauses->front());
  mergeVarList(existing, incoming);
  current_clauses->pop_back();
  directive->getClausesInOriginalOrder()->pop_back();
  delete incoming;
}

OpenACCClause *OpenACCDeleteClause::addClause(OpenACCDirective *directive) {
  auto *all_clauses = directive->getAllClauses();
  auto *current_clauses = directive->getClauses(ACCC_delete);
  OpenACCClause *new_clause = nullptr;
  if (current_clauses->size() == 0) {
    new_clause = new OpenACCDeleteClause();
    current_clauses = new std::vector<OpenACCClause *>();
    current_clauses->push_back(new_clause);
    (*all_clauses)[ACCC_delete] = current_clauses;
  } else if (OpenACCDirective::getClauseMerging()) {
    new_clause = current_clauses->at(0);
  } else {
    new_clause = new OpenACCDeleteClause();
    current_clauses->push_back(new_clause);
  }
  return new_clause;
}

void OpenACCDetachClause::mergeClause(OpenACCDirective *directive,
                                      OpenACCClause *current_clause) {
  if (!OpenACCDirective::getClauseMerging()) {
    return;
  }
  auto *current_clauses = directive->getClauses(ACCC_detach);
  if (current_clauses->size() < 2) {
    return;
  }
  auto *incoming = static_cast<OpenACCDetachClause *>(current_clause);
  auto *existing =
      static_cast<OpenACCDetachClause *>(current_clauses->front());
  mergeVarList(existing, incoming);
  current_clauses->pop_back();
  directive->getClausesInOriginalOrder()->pop_back();
  delete incoming;
}

OpenACCClause *OpenACCDetachClause::addClause(OpenACCDirective *directive) {
  auto *all_clauses = directive->getAllClauses();
  auto *current_clauses = directive->getClauses(ACCC_detach);
  OpenACCClause *new_clause = nullptr;
  if (current_clauses->size() == 0) {
    new_clause = new OpenACCDetachClause();
    current_clauses = new std::vector<OpenACCClause *>();
    current_clauses->push_back(new_clause);
    (*all_clauses)[ACCC_detach] = current_clauses;
  } else if (OpenACCDirective::getClauseMerging()) {
    new_clause = current_clauses->at(0);
  } else {
    new_clause = new OpenACCDetachClause();
    current_clauses->push_back(new_clause);
  }
  return new_clause;
}

void OpenACCCopyClause::mergeClause(OpenACCDirective *directive,
                                    OpenACCClause *current_clause) {
  (void)directive;
  (void)current_clause;
}

OpenACCClause *OpenACCCopyClause::addClause(OpenACCDirective *directive) {
  auto *all_clauses = directive->getAllClauses();
  auto *current_clauses = directive->getClauses(ACCC_copy);
  OpenACCClause *new_clause = nullptr;
  if (current_clauses->size() == 0) {
    new_clause = new OpenACCCopyClause();
    current_clauses = new std::vector<OpenACCClause *>();
    current_clauses->push_back(new_clause);
    (*all_clauses)[ACCC_copy] = current_clauses;
  } else if (OpenACCDirective::getClauseMerging()) {
    new_clause = current_clauses->at(0);
  } else {
    new_clause = new OpenACCCopyClause();
    current_clauses->push_back(new_clause);
  }
  return new_clause;
}

OpenACCClause *OpenACCCopyinClause::addClause(OpenACCDirective *directive) {

  std::map<OpenACCClauseKind, std::vector<OpenACCClause *> *> *all_clauses =
      directive->getAllClauses();
  std::vector<OpenACCClause *> *current_clauses =
      directive->getClauses(ACCC_copyin);
  OpenACCClause *new_clause = NULL;

  if (current_clauses->size() == 0) {
    new_clause = new OpenACCCopyinClause();
    current_clauses = new std::vector<OpenACCClause *>();
    current_clauses->push_back(new_clause);
    (*all_clauses)[ACCC_copyin] = current_clauses;
  } else {
    new_clause = new OpenACCCopyinClause();
    current_clauses->push_back(new_clause);
  };

  return new_clause;
};

void OpenACCCopyinClause::mergeClause(OpenACCDirective *directive,
                                      OpenACCClause *current_clause) {
  // Respect the global clause merging flag
  if (!OpenACCDirective::getClauseMerging()) {
    return;
  }

  std::vector<OpenACCClause *> *current_clauses =
      directive->getClauses(ACCC_copyin);
  for (std::vector<OpenACCClause *>::iterator it = current_clauses->begin();
       it != current_clauses->end() - 1; it++) {
    if (((OpenACCCopyinClause *)(*it))->getModifier() ==
            ((OpenACCCopyinClause *)current_clause)->getModifier() &&
        ((OpenACCClause *)(*it))->getOriginalKeyword() ==
            ((OpenACCClause *)current_clause)->getOriginalKeyword()) {
      auto *existing = static_cast<OpenACCCopyinClause *>(*it);
      auto *incoming = static_cast<OpenACCCopyinClause *>(current_clause);
      mergeVarList(existing, incoming);
      current_clauses->pop_back();
      directive->getClausesInOriginalOrder()->pop_back();
      break;
    }
  }
};

OpenACCClause *OpenACCCopyoutClause::addClause(OpenACCDirective *directive) {

  std::map<OpenACCClauseKind, std::vector<OpenACCClause *> *> *all_clauses =
      directive->getAllClauses();
  std::vector<OpenACCClause *> *current_clauses =
      directive->getClauses(ACCC_copyout);
  OpenACCClause *new_clause = NULL;

  if (current_clauses->size() == 0) {
    new_clause = new OpenACCCopyoutClause();
    current_clauses = new std::vector<OpenACCClause *>();
    current_clauses->push_back(new_clause);
    (*all_clauses)[ACCC_copyout] = current_clauses;
  } else {
    new_clause = new OpenACCCopyoutClause();
    current_clauses->push_back(new_clause);
  };

  return new_clause;
};

void OpenACCCopyoutClause::mergeClause(OpenACCDirective *directive,
                                       OpenACCClause *current_clause) {
  // Respect the global clause merging flag
  if (!OpenACCDirective::getClauseMerging()) {
    return;
  }

  std::vector<OpenACCClause *> *current_clauses =
      directive->getClauses(ACCC_copyout);
  for (std::vector<OpenACCClause *>::iterator it = current_clauses->begin();
       it != current_clauses->end() - 1; it++) {
    if (((OpenACCCopyoutClause *)(*it))->getModifier() ==
            ((OpenACCCopyoutClause *)current_clause)->getModifier() &&
        ((OpenACCClause *)(*it))->getOriginalKeyword() ==
            ((OpenACCClause *)current_clause)->getOriginalKeyword()) {
      auto *existing = static_cast<OpenACCCopyoutClause *>(*it);
      auto *incoming = static_cast<OpenACCCopyoutClause *>(current_clause);
      mergeVarList(existing, incoming);
      current_clauses->pop_back();
      directive->getClausesInOriginalOrder()->pop_back();
      break;
    }
  }
};

OpenACCClause *OpenACCCreateClause::addClause(OpenACCDirective *directive) {

  std::map<OpenACCClauseKind, std::vector<OpenACCClause *> *> *all_clauses =
      directive->getAllClauses();
  std::vector<OpenACCClause *> *current_clauses =
      directive->getClauses(ACCC_create);
  OpenACCClause *new_clause = NULL;

  if (current_clauses->size() == 0) {
    new_clause = new OpenACCCreateClause();
    current_clauses = new std::vector<OpenACCClause *>();
    current_clauses->push_back(new_clause);
    (*all_clauses)[ACCC_create] = current_clauses;
  } else {
    new_clause = new OpenACCCreateClause();
    current_clauses->push_back(new_clause);
  };

  return new_clause;
};

void OpenACCCreateClause::mergeClause(OpenACCDirective *directive,
                                      OpenACCClause *current_clause) {
  // Respect the global clause merging flag
  if (!OpenACCDirective::getClauseMerging()) {
    return;
  }

  std::vector<OpenACCClause *> *current_clauses =
      directive->getClauses(ACCC_create);
  for (std::vector<OpenACCClause *>::iterator it = current_clauses->begin();
       it != current_clauses->end() - 1; it++) {
    if (((OpenACCCreateClause *)(*it))->getModifier() ==
            ((OpenACCCreateClause *)current_clause)->getModifier() &&
        ((OpenACCClause *)(*it))->getOriginalKeyword() ==
            ((OpenACCClause *)current_clause)->getOriginalKeyword()) {
      auto *existing = static_cast<OpenACCCreateClause *>(*it);
      auto *incoming = static_cast<OpenACCCreateClause *>(current_clause);
      mergeVarList(existing, incoming);
      current_clauses->pop_back();
      directive->getClausesInOriginalOrder()->pop_back();
      break;
    }
  }
};

OpenACCClause *OpenACCNoCreateClause::addClause(OpenACCDirective *directive) {
  auto *all_clauses = directive->getAllClauses();
  auto *current_clauses = directive->getClauses(ACCC_no_create);
  OpenACCClause *new_clause = nullptr;
  if (current_clauses->size() == 0) {
    new_clause = new OpenACCNoCreateClause();
    current_clauses = new std::vector<OpenACCClause *>();
    current_clauses->push_back(new_clause);
    (*all_clauses)[ACCC_no_create] = current_clauses;
  } else if (OpenACCDirective::getClauseMerging()) {
    new_clause = current_clauses->at(0);
  } else {
    new_clause = new OpenACCNoCreateClause();
    current_clauses->push_back(new_clause);
  }
  return new_clause;
}

OpenACCClause *OpenACCPresentClause::addClause(OpenACCDirective *directive) {
  auto *all_clauses = directive->getAllClauses();
  auto *current_clauses = directive->getClauses(ACCC_present);
  OpenACCClause *new_clause = nullptr;
  if (current_clauses->size() == 0) {
    new_clause = new OpenACCPresentClause();
    current_clauses = new std::vector<OpenACCClause *>();
    current_clauses->push_back(new_clause);
    (*all_clauses)[ACCC_present] = current_clauses;
  } else if (OpenACCDirective::getClauseMerging()) {
    new_clause = current_clauses->at(0);
  } else {
    new_clause = new OpenACCPresentClause();
    current_clauses->push_back(new_clause);
  }
  return new_clause;
}

OpenACCClause *OpenACCLinkClause::addClause(OpenACCDirective *directive) {
  auto *all_clauses = directive->getAllClauses();
  auto *current_clauses = directive->getClauses(ACCC_link);
  OpenACCClause *new_clause = nullptr;
  if (current_clauses->size() == 0) {
    new_clause = new OpenACCLinkClause();
    current_clauses = new std::vector<OpenACCClause *>();
    current_clauses->push_back(new_clause);
    (*all_clauses)[ACCC_link] = current_clauses;
  } else if (OpenACCDirective::getClauseMerging()) {
    new_clause = current_clauses->at(0);
  } else {
    new_clause = new OpenACCLinkClause();
    current_clauses->push_back(new_clause);
  }
  return new_clause;
}

OpenACCClause *
OpenACCDeviceResidentClause::addClause(OpenACCDirective *directive) {
  auto *all_clauses = directive->getAllClauses();
  auto *current_clauses = directive->getClauses(ACCC_device_resident);
  OpenACCClause *new_clause = nullptr;
  if (current_clauses->size() == 0) {
    new_clause = new OpenACCDeviceResidentClause();
    current_clauses = new std::vector<OpenACCClause *>();
    current_clauses->push_back(new_clause);
    (*all_clauses)[ACCC_device_resident] = current_clauses;
  } else if (OpenACCDirective::getClauseMerging()) {
    new_clause = current_clauses->at(0);
  } else {
    new_clause = new OpenACCDeviceResidentClause();
    current_clauses->push_back(new_clause);
  }
  return new_clause;
}

void OpenACCFirstprivateClause::mergeClause(OpenACCDirective *directive,
                                            OpenACCClause *current_clause) {
  if (!OpenACCDirective::getClauseMerging()) {
    return;
  }
  auto *current_clauses = directive->getClauses(ACCC_firstprivate);
  if (current_clauses->size() < 2) {
    return;
  }
  auto *incoming = static_cast<OpenACCFirstprivateClause *>(current_clause);
  auto *existing = static_cast<OpenACCFirstprivateClause *>(
      current_clauses->front());
  mergeVarList(existing, incoming);
  current_clauses->pop_back();
  directive->getClausesInOriginalOrder()->pop_back();
  delete incoming;
}

OpenACCClause *
OpenACCFirstprivateClause::addClause(OpenACCDirective *directive) {
  auto *all_clauses = directive->getAllClauses();
  auto *current_clauses = directive->getClauses(ACCC_firstprivate);
  OpenACCClause *new_clause = nullptr;
  if (current_clauses->size() == 0) {
    new_clause = new OpenACCFirstprivateClause();
    current_clauses = new std::vector<OpenACCClause *>();
    current_clauses->push_back(new_clause);
    (*all_clauses)[ACCC_firstprivate] = current_clauses;
  } else if (OpenACCDirective::getClauseMerging()) {
    new_clause = current_clauses->at(0);
  } else {
    new_clause = new OpenACCFirstprivateClause();
    current_clauses->push_back(new_clause);
  }
  return new_clause;
}

void OpenACCHostClause::mergeClause(OpenACCDirective *directive,
                                    OpenACCClause *current_clause) {
  if (!OpenACCDirective::getClauseMerging()) {
    return;
  }
  auto *current_clauses = directive->getClauses(ACCC_host);
  if (current_clauses->size() < 2) {
    return;
  }
  auto *incoming = static_cast<OpenACCHostClause *>(current_clause);
  auto *existing =
      static_cast<OpenACCHostClause *>(current_clauses->front());
  mergeVarList(existing, incoming);
  current_clauses->pop_back();
  directive->getClausesInOriginalOrder()->pop_back();
  delete incoming;
}

OpenACCClause *OpenACCHostClause::addClause(OpenACCDirective *directive) {
  auto *all_clauses = directive->getAllClauses();
  auto *current_clauses = directive->getClauses(ACCC_host);
  OpenACCClause *new_clause = nullptr;
  if (current_clauses->size() == 0) {
    new_clause = new OpenACCHostClause();
    current_clauses = new std::vector<OpenACCClause *>();
    current_clauses->push_back(new_clause);
    (*all_clauses)[ACCC_host] = current_clauses;
  } else if (OpenACCDirective::getClauseMerging()) {
    new_clause = current_clauses->at(0);
  } else {
    new_clause = new OpenACCHostClause();
    current_clauses->push_back(new_clause);
  }
  return new_clause;
}

void OpenACCPrivateClause::mergeClause(OpenACCDirective *directive,
                                       OpenACCClause *current_clause) {
  if (!OpenACCDirective::getClauseMerging()) {
    return;
  }
  auto *current_clauses = directive->getClauses(ACCC_private);
  if (current_clauses->size() < 2) {
    return;
  }
  auto *incoming = static_cast<OpenACCPrivateClause *>(current_clause);
  auto *existing =
      static_cast<OpenACCPrivateClause *>(current_clauses->front());
  mergeVarList(existing, incoming);
  current_clauses->pop_back();
  directive->getClausesInOriginalOrder()->pop_back();
  delete incoming;
}

OpenACCClause *OpenACCPrivateClause::addClause(OpenACCDirective *directive) {
  auto *all_clauses = directive->getAllClauses();
  auto *current_clauses = directive->getClauses(ACCC_private);
  OpenACCClause *new_clause = nullptr;
  if (current_clauses->size() == 0) {
    new_clause = new OpenACCPrivateClause();
    current_clauses = new std::vector<OpenACCClause *>();
    current_clauses->push_back(new_clause);
    (*all_clauses)[ACCC_private] = current_clauses;
  } else if (OpenACCDirective::getClauseMerging()) {
    new_clause = current_clauses->at(0);
  } else {
    new_clause = new OpenACCPrivateClause();
    current_clauses->push_back(new_clause);
  }
  return new_clause;
}

OpenACCClause *OpenACCSelfClause::addClause(OpenACCDirective *directive) {
  auto *all_clauses = directive->getAllClauses();
  auto *current_clauses = directive->getClauses(ACCC_self);
  OpenACCClause *new_clause = nullptr;
  if (current_clauses->size() == 0) {
    current_clauses = new std::vector<OpenACCClause *>();
    (*all_clauses)[ACCC_self] = current_clauses;
  }
  new_clause = new OpenACCSelfClause();
  current_clauses->push_back(new_clause);
  return new_clause;
}

OpenACCClause *OpenACCDeviceptrClause::addClause(OpenACCDirective *directive) {
  auto *all_clauses = directive->getAllClauses();
  auto *current_clauses = directive->getClauses(ACCC_deviceptr);
  OpenACCClause *new_clause = nullptr;
  if (current_clauses->size() == 0) {
    new_clause = new OpenACCDeviceptrClause();
    current_clauses = new std::vector<OpenACCClause *>();
    current_clauses->push_back(new_clause);
    (*all_clauses)[ACCC_deviceptr] = current_clauses;
  } else if (OpenACCDirective::getClauseMerging()) {
    new_clause = current_clauses->at(0);
  } else {
    new_clause = new OpenACCDeviceptrClause();
    current_clauses->push_back(new_clause);
  }
  return new_clause;
}

OpenACCClause *OpenACCUseDeviceClause::addClause(OpenACCDirective *directive) {
  auto *all_clauses = directive->getAllClauses();
  auto *current_clauses = directive->getClauses(ACCC_use_device);
  OpenACCClause *new_clause = nullptr;
  if (current_clauses->size() == 0) {
    new_clause = new OpenACCUseDeviceClause();
    current_clauses = new std::vector<OpenACCClause *>();
    current_clauses->push_back(new_clause);
    (*all_clauses)[ACCC_use_device] = current_clauses;
  } else {
    new_clause = new OpenACCUseDeviceClause();
    current_clauses->push_back(new_clause);
  }
  return new_clause;
}

void OpenACCNoCreateClause::mergeClause(OpenACCDirective *directive,
                                        OpenACCClause *current_clause) {
  if (!OpenACCDirective::getClauseMerging()) {
    return;
  }
  auto *current_clauses = directive->getClauses(ACCC_no_create);
  if (current_clauses->size() < 2) {
    return;
  }
  auto *incoming = static_cast<OpenACCNoCreateClause *>(current_clause);
  auto *existing =
      static_cast<OpenACCNoCreateClause *>(current_clauses->front());
  mergeVarList(existing, incoming);
  current_clauses->pop_back();
  directive->getClausesInOriginalOrder()->pop_back();
  delete incoming;
}

void OpenACCPresentClause::mergeClause(OpenACCDirective *directive,
                                       OpenACCClause *current_clause) {
  if (!OpenACCDirective::getClauseMerging()) {
    return;
  }
  auto *current_clauses = directive->getClauses(ACCC_present);
  if (current_clauses->size() < 2) {
    return;
  }
  auto *incoming = static_cast<OpenACCPresentClause *>(current_clause);
  auto *existing =
      static_cast<OpenACCPresentClause *>(current_clauses->front());
  mergeVarList(existing, incoming);
  current_clauses->pop_back();
  directive->getClausesInOriginalOrder()->pop_back();
  delete incoming;
}

void OpenACCLinkClause::mergeClause(OpenACCDirective *directive,
                                    OpenACCClause *current_clause) {
  if (!OpenACCDirective::getClauseMerging()) {
    return;
  }
  auto *current_clauses = directive->getClauses(ACCC_link);
  if (current_clauses->size() < 2) {
    return;
  }
  auto *incoming = static_cast<OpenACCLinkClause *>(current_clause);
  auto *existing =
      static_cast<OpenACCLinkClause *>(current_clauses->front());
  mergeVarList(existing, incoming);
  current_clauses->pop_back();
  directive->getClausesInOriginalOrder()->pop_back();
  delete incoming;
}

void OpenACCDeviceResidentClause::mergeClause(OpenACCDirective *directive,
                                              OpenACCClause *current_clause) {
  if (!OpenACCDirective::getClauseMerging()) {
    return;
  }
  auto *current_clauses = directive->getClauses(ACCC_device_resident);
  if (current_clauses->size() < 2) {
    return;
  }
  auto *incoming = static_cast<OpenACCDeviceResidentClause *>(current_clause);
  auto *existing = static_cast<OpenACCDeviceResidentClause *>(
      current_clauses->front());
  mergeVarList(existing, incoming);
  current_clauses->pop_back();
  directive->getClausesInOriginalOrder()->pop_back();
  delete incoming;
}

void OpenACCDeviceptrClause::mergeClause(OpenACCDirective *directive,
                                         OpenACCClause *current_clause) {
  if (!OpenACCDirective::getClauseMerging()) {
    return;
  }
  auto *current_clauses = directive->getClauses(ACCC_deviceptr);
  if (current_clauses->size() < 2) {
    return;
  }
  auto *incoming = static_cast<OpenACCDeviceptrClause *>(current_clause);
  auto *existing =
      static_cast<OpenACCDeviceptrClause *>(current_clauses->front());
  mergeVarList(existing, incoming);
  current_clauses->pop_back();
  directive->getClausesInOriginalOrder()->pop_back();
  delete incoming;
}

void OpenACCUseDeviceClause::mergeClause(OpenACCDirective *directive,
                                         OpenACCClause *current_clause) {
  if (!OpenACCDirective::getClauseMerging()) {
    return;
  }
  auto *current_clauses = directive->getClauses(ACCC_use_device);
  if (current_clauses->size() < 2) {
    return;
  }
  auto *incoming = static_cast<OpenACCUseDeviceClause *>(current_clause);
  auto *existing =
      static_cast<OpenACCUseDeviceClause *>(current_clauses->front());
  mergeVarList(existing, incoming);
  current_clauses->pop_back();
  directive->getClausesInOriginalOrder()->pop_back();
  delete incoming;
}

OpenACCClause *OpenACCDefaultClause::addClause(OpenACCDirective *directive) {

  std::map<OpenACCClauseKind, std::vector<OpenACCClause *> *> *all_clauses =
      directive->getAllClauses();
  std::vector<OpenACCClause *> *current_clauses =
      directive->getClauses(ACCC_default);
  OpenACCClause *new_clause = NULL;

  if (current_clauses->size() == 0) {
    new_clause = new OpenACCDefaultClause();
    current_clauses = new std::vector<OpenACCClause *>();
    current_clauses->push_back(new_clause);
    (*all_clauses)[ACCC_default] = current_clauses;
  } else { /* could be an error since default clause may only appear once */
    std::cerr << "Cannot have two default clause for the directive "
              << directive->getKind() << ", ignored\n";
  };

  return new_clause;
};

void OpenACCDefaultAsyncClause::mergeClause(OpenACCDirective *directive,
                                            OpenACCClause *current_clause) {
  // Respect the global clause merging flag
  if (!OpenACCDirective::getClauseMerging()) {
    return;
  }

  std::vector<OpenACCClause *> *current_clauses =
      directive->getClauses(ACCC_default_async);

  if (current_clauses->size() < 2) {
    return;
  }

  auto *incoming = static_cast<OpenACCDefaultAsyncClause *>(current_clause);
  for (auto it = current_clauses->begin(); it != current_clauses->end() - 1;
       ++it) {
    auto *existing = static_cast<OpenACCDefaultAsyncClause *>(*it);
    if (incoming->getAsyncExpr().text == existing->getAsyncExpr().text) {
      current_clauses->pop_back();
      directive->getClausesInOriginalOrder()->pop_back();
      delete incoming;
      break;
    }
  }
};

OpenACCClause *OpenACCDefaultAsyncClause::addClause(
    OpenACCDirective *directive) {

  std::map<OpenACCClauseKind, std::vector<OpenACCClause *> *> *all_clauses =
      directive->getAllClauses();
  std::vector<OpenACCClause *> *current_clauses =
      directive->getClauses(ACCC_default_async);
  OpenACCClause *new_clause = NULL;
  if (current_clauses->size() == 0) {
    new_clause = new OpenACCDefaultAsyncClause();
    current_clauses = new std::vector<OpenACCClause *>();
    current_clauses->push_back(new_clause);
    (*all_clauses)[ACCC_default_async] = current_clauses;
  } else {
    new_clause = new OpenACCDefaultAsyncClause();
    current_clauses->push_back(new_clause);
  }
  return new_clause;
}

void OpenACCDeviceClause::mergeClause(OpenACCDirective *directive,
                                      OpenACCClause *current_clause) {
  // Respect the global clause merging flag
  if (!OpenACCDirective::getClauseMerging()) {
    return;
  }

  auto *current_clauses = directive->getClauses(ACCC_device);
  if (current_clauses->size() < 2) {
    return;
  }

  auto *incoming = static_cast<OpenACCDeviceClause *>(current_clause);
  auto *existing = static_cast<OpenACCDeviceClause *>(current_clauses->front());

  for (const auto &dev : incoming->getDevices()) {
    bool found = false;
    for (const auto &prev : existing->getDevices()) {
      if (prev.text == dev.text) {
        found = true;
        break;
      }
    }
    if (!found) {
      existing->addDevice(dev);
    }
  }

  current_clauses->pop_back();
  directive->getClausesInOriginalOrder()->pop_back();
  delete incoming;
}

OpenACCClause *OpenACCDeviceClause::addClause(OpenACCDirective *directive) {

  std::map<OpenACCClauseKind, std::vector<OpenACCClause *> *> *all_clauses =
      directive->getAllClauses();
  std::vector<OpenACCClause *> *current_clauses =
      directive->getClauses(ACCC_device);
  OpenACCClause *new_clause = NULL;
  if (current_clauses->size() == 0) {
    new_clause = new OpenACCDeviceClause();
    current_clauses = new std::vector<OpenACCClause *>();
    current_clauses->push_back(new_clause);
    (*all_clauses)[ACCC_device] = current_clauses;
  } else if (OpenACCDirective::getClauseMerging()) {
    new_clause = current_clauses->at(0);
  } else {
    new_clause = new OpenACCDeviceClause();
    current_clauses->push_back(new_clause);
  }
  return new_clause;
}

void OpenACCIfClause::mergeClause(OpenACCDirective *directive,
                                  OpenACCClause *current_clause) {
  // Respect the global clause merging flag
  if (!OpenACCDirective::getClauseMerging()) {
    return;
  }

  auto *current_clauses = directive->getClauses(ACCC_if);
  if (current_clauses->size() < 2) {
    return;
  }

  auto *incoming = static_cast<OpenACCIfClause *>(current_clause);
  auto *existing = static_cast<OpenACCIfClause *>(current_clauses->front());

  if (!incoming->getCondition().text.empty() &&
      existing->getCondition().text.empty()) {
    existing->setCondition(incoming->getCondition());
  }

  current_clauses->pop_back();
  directive->getClausesInOriginalOrder()->pop_back();
  delete incoming;
}

OpenACCClause *OpenACCIfClause::addClause(OpenACCDirective *directive) {

  std::map<OpenACCClauseKind, std::vector<OpenACCClause *> *> *all_clauses =
      directive->getAllClauses();
  std::vector<OpenACCClause *> *current_clauses =
      directive->getClauses(ACCC_if);
  OpenACCClause *new_clause = NULL;
  if (current_clauses->size() == 0) {
    new_clause = new OpenACCIfClause();
    current_clauses = new std::vector<OpenACCClause *>();
    current_clauses->push_back(new_clause);
    (*all_clauses)[ACCC_if] = current_clauses;
  } else if (OpenACCDirective::getClauseMerging()) {
    new_clause = current_clauses->at(0);
  } else {
    new_clause = new OpenACCIfClause();
    current_clauses->push_back(new_clause);
  }
  return new_clause;
}

void OpenACCDeviceNumClause::mergeClause(OpenACCDirective *directive,
                                         OpenACCClause *current_clause) {
  // Respect the global clause merging flag
  if (!OpenACCDirective::getClauseMerging()) {
    return;
  }

  std::vector<OpenACCClause *> *current_clauses =
      directive->getClauses(ACCC_device_num);

  for (std::vector<OpenACCClause *>::iterator it = current_clauses->begin();
       it != current_clauses->end() - 1; it++) {
    auto *existing = static_cast<OpenACCDeviceNumClause *>(*it);
    auto *incoming = static_cast<OpenACCDeviceNumClause *>(current_clause);
    if (existing->getDeviceExpr().text == incoming->getDeviceExpr().text) {
      current_clauses->pop_back();
      directive->getClausesInOriginalOrder()->pop_back();
      delete incoming;
      break;
    }
  }
};

OpenACCClause *OpenACCDeviceNumClause::addClause(OpenACCDirective *directive) {

  std::map<OpenACCClauseKind, std::vector<OpenACCClause *> *> *all_clauses =
      directive->getAllClauses();
  std::vector<OpenACCClause *> *current_clauses =
      directive->getClauses(ACCC_device_num);
  OpenACCClause *new_clause = NULL;
  if (current_clauses->size() == 0) {
    new_clause = new OpenACCDeviceNumClause();
    current_clauses = new std::vector<OpenACCClause *>();
    current_clauses->push_back(new_clause);
    (*all_clauses)[ACCC_device_num] = current_clauses;
  } else {
    new_clause = new OpenACCDeviceNumClause();
    current_clauses->push_back(new_clause);
  }
  return new_clause;
}

OpenACCClause *OpenACCTileClause::addClause(OpenACCDirective *directive) {

  std::map<OpenACCClauseKind, std::vector<OpenACCClause *> *> *all_clauses =
      directive->getAllClauses();
  std::vector<OpenACCClause *> *current_clauses =
      directive->getClauses(ACCC_tile);
  OpenACCClause *new_clause = NULL;
  if (current_clauses->size() == 0) {
    new_clause = new OpenACCTileClause();
    current_clauses = new std::vector<OpenACCClause *>();
    current_clauses->push_back(new_clause);
    (*all_clauses)[ACCC_tile] = current_clauses;
  } else {
    new_clause = new OpenACCTileClause();
    current_clauses->push_back(new_clause);
  }
  return new_clause;
}

void OpenACCTileClause::mergeClause(OpenACCDirective *directive,
                                    OpenACCClause *current_clause) {
  // Respect the global clause merging flag
  if (!OpenACCDirective::getClauseMerging()) {
    return;
  }

  auto *current_clauses = directive->getClauses(ACCC_tile);
  if (current_clauses->size() < 2) {
    return;
  }

  auto *incoming = static_cast<OpenACCTileClause *>(current_clause);
  for (auto it = current_clauses->begin(); it != current_clauses->end() - 1;
       ++it) {
    auto *existing = static_cast<OpenACCTileClause *>(*it);
    for (const auto &size : incoming->getTileSizes()) {
      existing->addTileSize(size);
    }
    current_clauses->pop_back();
    directive->getClausesInOriginalOrder()->pop_back();
    delete incoming;
    break;
  }
}

OpenACCClause *OpenACCCollapseClause::addClause(OpenACCDirective *directive) {

  std::map<OpenACCClauseKind, std::vector<OpenACCClause *> *> *all_clauses =
      directive->getAllClauses();
  std::vector<OpenACCClause *> *current_clauses =
      directive->getClauses(ACCC_collapse);
  OpenACCClause *new_clause = NULL;
  if (current_clauses->size() == 0) {
    new_clause = new OpenACCCollapseClause();
    current_clauses = new std::vector<OpenACCClause *>();
    current_clauses->push_back(new_clause);
    (*all_clauses)[ACCC_collapse] = current_clauses;
  } else {
    new_clause = new OpenACCCollapseClause();
    current_clauses->push_back(new_clause);
  }
  return new_clause;
}

OpenACCClause *OpenACCGangClause::addClause(OpenACCDirective *directive) {

  std::map<OpenACCClauseKind, std::vector<OpenACCClause *> *> *all_clauses =
      directive->getAllClauses();
  std::vector<OpenACCClause *> *current_clauses =
      directive->getClauses(ACCC_gang);
  OpenACCGangClause *new_clause = nullptr;
  if (directive->getKind() == ACCD_routine && current_clauses->size() != 0) {
    new_clause = static_cast<OpenACCGangClause *>(current_clauses->at(0));
  } else {
    new_clause = new OpenACCGangClause();
    if (current_clauses->size() == 0) {
      current_clauses = new std::vector<OpenACCClause *>();
      (*all_clauses)[ACCC_gang] = current_clauses;
    }
    current_clauses->push_back(new_clause);
  }
  return new_clause;
}

void OpenACCGangClause::mergeClause(OpenACCDirective *directive,
                                    OpenACCClause *current_clause) {
  // Respect the global clause merging flag
  if (!OpenACCDirective::getClauseMerging()) {
    return;
  }

  std::vector<OpenACCClause *> *current_clauses =
      directive->getClauses(ACCC_gang);

  if (current_clauses->size() < 2) {
    return;
  }

  auto *incoming = static_cast<OpenACCGangClause *>(current_clause);
  for (auto it = current_clauses->begin(); it != current_clauses->end() - 1;
       ++it) {
    auto *existing = static_cast<OpenACCGangClause *>(*it);

    bool incoming_empty = incoming->getArgs().empty();
    bool existing_empty = existing->getArgs().empty();

    // Merge empty clauses
    if (incoming_empty && existing_empty) {
      current_clauses->pop_back();
      directive->getClausesInOriginalOrder()->pop_back();
      delete incoming;
      break;
    }

    if (!incoming_empty && !existing_empty) {
      for (const auto &arg : incoming->getArgs()) {
        bool found = false;
        for (const auto &prev : existing->getArgs()) {
          if (prev.kind == arg.kind && prev.value.text == arg.value.text) {
            found = true;
            break;
          }
        }
        if (!found) {
          existing->addArg(arg.kind, arg.value);
        }
      }
      current_clauses->pop_back();
      directive->getClausesInOriginalOrder()->pop_back();
      delete incoming;
      break;
    }
  }
};

void OpenACCNumGangsClause::mergeClause(OpenACCDirective *directive,
                                        OpenACCClause *current_clause) {
  // Respect the global clause merging flag
  if (!OpenACCDirective::getClauseMerging()) {
    return;
  }

  std::vector<OpenACCClause *> *current_clauses =
      directive->getClauses(ACCC_num_gangs);

  if (current_clauses->size() < 2) {
    return;
  }

  auto *incoming = static_cast<OpenACCNumGangsClause *>(current_clause);
  for (auto it = current_clauses->begin(); it != current_clauses->end() - 1;
       ++it) {
    auto *existing = static_cast<OpenACCNumGangsClause *>(*it);
    const auto &existing_nums = existing->getNums();
    const auto &incoming_nums = incoming->getNums();
    bool same = existing_nums.size() == incoming_nums.size();
    if (same) {
      for (size_t idx = 0; idx < existing_nums.size(); ++idx) {
        if (existing_nums[idx].text != incoming_nums[idx].text) {
          same = false;
          break;
        }
      }
    }
    if (same) {
      current_clauses->pop_back();
      directive->getClausesInOriginalOrder()->pop_back();
      delete incoming;
      break;
    }
  }
};

OpenACCClause *OpenACCNumGangsClause::addClause(OpenACCDirective *directive) {

  std::map<OpenACCClauseKind, std::vector<OpenACCClause *> *> *all_clauses =
      directive->getAllClauses();
  std::vector<OpenACCClause *> *current_clauses =
      directive->getClauses(ACCC_num_gangs);
  OpenACCClause *new_clause = NULL;

  if (current_clauses->size() == 0) {
    new_clause = new OpenACCNumGangsClause();
    current_clauses = new std::vector<OpenACCClause *>();
    current_clauses->push_back(new_clause);
    (*all_clauses)[ACCC_num_gangs] = current_clauses;
  } else {
    new_clause = new OpenACCNumGangsClause();
    current_clauses->push_back(new_clause);
  }
  return new_clause;
}

void OpenACCNumWorkersClause::mergeClause(OpenACCDirective *directive,
                                          OpenACCClause *current_clause) {
  // Respect the global clause merging flag
  if (!OpenACCDirective::getClauseMerging()) {
    return;
  }

  std::vector<OpenACCClause *> *current_clauses =
      directive->getClauses(ACCC_num_workers);

  if (current_clauses->size() < 2) {
    return;
  }

  auto *incoming = static_cast<OpenACCNumWorkersClause *>(current_clause);
  for (auto it = current_clauses->begin(); it != current_clauses->end() - 1;
       ++it) {
    auto *existing = static_cast<OpenACCNumWorkersClause *>(*it);
    if (existing->getNumExpr().text == incoming->getNumExpr().text) {
      current_clauses->pop_back();
      directive->getClausesInOriginalOrder()->pop_back();
      delete incoming;
      break;
    }
  }
};

OpenACCClause *OpenACCNumWorkersClause::addClause(OpenACCDirective *directive) {

  std::map<OpenACCClauseKind, std::vector<OpenACCClause *> *> *all_clauses =
      directive->getAllClauses();
  std::vector<OpenACCClause *> *current_clauses =
      directive->getClauses(ACCC_num_workers);
  OpenACCClause *new_clause = NULL;
  if (directive->getKind() == ACCD_routine && current_clauses->size() != 0) {
    new_clause = current_clauses->at(0);
  } else {
    new_clause = new OpenACCNumWorkersClause();
    if (current_clauses->size() == 0) {
      current_clauses = new std::vector<OpenACCClause *>();
      (*all_clauses)[ACCC_num_workers] = current_clauses;
    }
    current_clauses->push_back(new_clause);
  }

  return new_clause;
}

OpenACCClause *OpenACCReductionClause::addClause(OpenACCDirective *directive) {

  std::map<OpenACCClauseKind, std::vector<OpenACCClause *> *> *all_clauses =
      directive->getAllClauses();
  std::vector<OpenACCClause *> *current_clauses =
      directive->getClauses(ACCC_reduction);
  OpenACCClause *new_clause = NULL;

  if (current_clauses->size() == 0) {
    new_clause = new OpenACCReductionClause();
    current_clauses = new std::vector<OpenACCClause *>();
    current_clauses->push_back(new_clause);
    (*all_clauses)[ACCC_reduction] = current_clauses;
  } else {
    new_clause = new OpenACCReductionClause();
    current_clauses->push_back(new_clause);
  };

  return new_clause;
};

void OpenACCReductionClause::mergeClause(OpenACCDirective *directive,
                                         OpenACCClause *current_clause) {
  // Respect the global clause merging flag
  if (!OpenACCDirective::getClauseMerging()) {
    return;
  }

  std::vector<OpenACCClause *> *current_clauses =
      directive->getClauses(ACCC_reduction);
  for (std::vector<OpenACCClause *>::iterator it = current_clauses->begin();
       it != current_clauses->end() - 1; it++) {
    if (((OpenACCReductionClause *)(*it))->getOperator() ==
        ((OpenACCReductionClause *)current_clause)->getOperator()) {
      auto *existing = static_cast<OpenACCReductionClause *>(*it);
      auto *incoming = static_cast<OpenACCReductionClause *>(current_clause);
      mergeVarList(existing, incoming);
      current_clauses->pop_back();
      directive->getClausesInOriginalOrder()->pop_back();
      break;
    }
  }
};

void OpenACCSelfClause::mergeClause(OpenACCDirective *directive,
                                    OpenACCClause *current_clause) {
  if (!OpenACCDirective::getClauseMerging()) {
    return;
  }

  auto *current_clauses = directive->getClauses(ACCC_self);
  if (current_clauses->size() < 2) {
    return;
  }

  auto *incoming = static_cast<OpenACCSelfClause *>(current_clause);
  const bool incoming_has_condition = !incoming->getCondition().text.empty();

  for (auto it = current_clauses->begin(); it != current_clauses->end() - 1;
       ++it) {
    auto *existing = static_cast<OpenACCSelfClause *>(*it);
    const bool existing_has_condition = !existing->getCondition().text.empty();

    if (incoming_has_condition && existing_has_condition) {
      if (incoming->getCondition().text == existing->getCondition().text) {
        current_clauses->pop_back();
        directive->getClausesInOriginalOrder()->pop_back();
        delete incoming;
        break;
      }
      continue;
    }

    // Merge self clauses that carry variable lists or are empty.
    if (!incoming_has_condition && !existing_has_condition) {
      const auto &incoming_vars = incoming->getVars();
      const auto &existing_vars = existing->getVars();
      const bool both_empty = incoming_vars.empty() && existing_vars.empty();
      if (both_empty) {
        current_clauses->pop_back();
        directive->getClausesInOriginalOrder()->pop_back();
        delete incoming;
        break;
      }

      for (const auto &var : incoming_vars) {
        existing->addVar(var);
      }
      current_clauses->pop_back();
      directive->getClausesInOriginalOrder()->pop_back();
      delete incoming;
      break;
    }
  }
};

OpenACCClause *OpenACCVectorClause::addClause(OpenACCDirective *directive) {

  std::map<OpenACCClauseKind, std::vector<OpenACCClause *> *> *all_clauses =
      directive->getAllClauses();
  std::vector<OpenACCClause *> *current_clauses =
      directive->getClauses(ACCC_vector);
  OpenACCClause *new_clause = NULL;
  if (directive->getKind() == ACCD_routine) {
    if (current_clauses->size() == 0) {
      new_clause = new OpenACCVectorClause();
      current_clauses = new std::vector<OpenACCClause *>();
      current_clauses->push_back(new_clause);
      (*all_clauses)[ACCC_vector] = current_clauses;
    } else {
      new_clause = current_clauses->at(0);
    }
  } else {
    if (current_clauses->size() == 0) {
      new_clause = new OpenACCVectorClause();
      current_clauses = new std::vector<OpenACCClause *>();
      current_clauses->push_back(new_clause);
      (*all_clauses)[ACCC_vector] = current_clauses;
    } else {
      new_clause = new OpenACCVectorClause();
      current_clauses->push_back(new_clause);
    };
  }

  return new_clause;
};

void OpenACCVectorClause::mergeClause(OpenACCDirective *directive,
                                      OpenACCClause *current_clause) {
  // Respect the global clause merging flag
  if (!OpenACCDirective::getClauseMerging()) {
    return;
  }

  std::vector<OpenACCClause *> *current_clauses =
      directive->getClauses(ACCC_vector);
  if (current_clauses->size() < 2) {
    return;
  }
  auto *incoming = static_cast<OpenACCVectorClause *>(current_clause);
  for (auto it = current_clauses->begin(); it != current_clauses->end() - 1;
       ++it) {
    auto *existing = static_cast<OpenACCVectorClause *>(*it);
    if (existing->getModifier() == incoming->getModifier() &&
        existing->getLengthExpr().text == incoming->getLengthExpr().text) {
      current_clauses->pop_back();
      directive->getClausesInOriginalOrder()->pop_back();
      delete incoming;
      break;
    }
  }
};

OpenACCClause *OpenACCVectorLengthClause::addClause(
    OpenACCDirective *directive) {

  std::map<OpenACCClauseKind, std::vector<OpenACCClause *> *> *all_clauses =
      directive->getAllClauses();
  std::vector<OpenACCClause *> *current_clauses =
      directive->getClauses(ACCC_vector_length);
  OpenACCClause *new_clause = NULL;
  if (directive->getKind() == ACCD_routine) {
    if (current_clauses->size() == 0) {
      new_clause = new OpenACCVectorLengthClause();
      current_clauses = new std::vector<OpenACCClause *>();
      current_clauses->push_back(new_clause);
      (*all_clauses)[ACCC_vector_length] = current_clauses;
    } else {
      new_clause = current_clauses->at(0);
    }
  } else {
    if (current_clauses->size() == 0) {
      new_clause = new OpenACCVectorLengthClause();
      current_clauses = new std::vector<OpenACCClause *>();
      current_clauses->push_back(new_clause);
      (*all_clauses)[ACCC_vector_length] = current_clauses;
    } else {
      new_clause = new OpenACCVectorLengthClause();
      current_clauses->push_back(new_clause);
    };
  }

  return new_clause;
}

void OpenACCVectorLengthClause::mergeClause(OpenACCDirective *directive,
                                            OpenACCClause *current_clause) {
  // Respect the global clause merging flag
  if (!OpenACCDirective::getClauseMerging()) {
    return;
  }

  std::vector<OpenACCClause *> *current_clauses =
      directive->getClauses(ACCC_vector_length);

  if (current_clauses->size() < 2) {
    return;
  }

  auto *incoming = static_cast<OpenACCVectorLengthClause *>(current_clause);
  for (auto it = current_clauses->begin(); it != current_clauses->end() - 1;
       ++it) {
    auto *existing = static_cast<OpenACCVectorLengthClause *>(*it);
    if (existing->getLengthExpr().text == incoming->getLengthExpr().text) {
      current_clauses->pop_back();
      directive->getClausesInOriginalOrder()->pop_back();
      delete incoming;
      break;
    }
  }
};

OpenACCClause *OpenACCWaitClause::addClause(OpenACCDirective *directive) {

  std::map<OpenACCClauseKind, std::vector<OpenACCClause *> *> *all_clauses =
      directive->getAllClauses();
  std::vector<OpenACCClause *> *current_clauses =
      directive->getClauses(ACCC_wait);
  OpenACCClause *new_clause = NULL;

  if (current_clauses->size() == 0) {
    current_clauses = new std::vector<OpenACCClause *>();
    (*all_clauses)[ACCC_wait] = current_clauses;
  };
  new_clause = new OpenACCWaitClause();
  current_clauses->push_back(new_clause);

  return new_clause;
};

void OpenACCWaitClause::mergeClause(OpenACCDirective *directive,
                                    OpenACCClause *current_clause) {
  // Respect the global clause merging flag
  if (!OpenACCDirective::getClauseMerging()) {
    return;
  }

  std::vector<OpenACCClause *> *current_clauses =
      directive->getClauses(ACCC_wait);

  if (current_clauses->size() < 2) {
    return;
  }

  auto *incoming = static_cast<OpenACCWaitClause *>(current_clause);
  for (auto it = current_clauses->begin(); it != current_clauses->end() - 1;
       ++it) {
    auto *existing = static_cast<OpenACCWaitClause *>(*it);
    if (incoming->getDevnum().text == existing->getDevnum().text &&
        incoming->getQueues() == existing->getQueues()) {
      const auto &incoming_ids = incoming->getAsyncIds();
      const auto &existing_ids = existing->getAsyncIds();
      if (incoming_ids.empty() && existing_ids.empty()) {
        current_clauses->pop_back();
        directive->getClausesInOriginalOrder()->pop_back();
        delete incoming;
        break;
      }
      if (!incoming_ids.empty() && !existing_ids.empty()) {
        for (const auto &id : incoming_ids) {
          bool found = false;
          for (const auto &prev : existing_ids) {
            if (prev.text == id.text) {
              found = true;
              break;
            }
          }
          if (!found) {
            existing->addAsyncId(id);
          }
        }
        current_clauses->pop_back();
        directive->getClausesInOriginalOrder()->pop_back();
        delete incoming;
        break;
      }
    }
  }
};

OpenACCClause *OpenACCWorkerClause::addClause(OpenACCDirective *directive) {

  std::map<OpenACCClauseKind, std::vector<OpenACCClause *> *> *all_clauses =
      directive->getAllClauses();
  std::vector<OpenACCClause *> *current_clauses =
      directive->getClauses(ACCC_worker);
  OpenACCClause *new_clause = NULL;
  if (directive->getKind() == ACCD_routine) {
    if (current_clauses->size() == 0) {
      new_clause = new OpenACCWorkerClause();
      current_clauses = new std::vector<OpenACCClause *>();
      current_clauses->push_back(new_clause);
      (*all_clauses)[ACCC_worker] = current_clauses;
    } else {
      new_clause = current_clauses->at(0);
    }
  } else {
    if (current_clauses->size() == 0) {
      new_clause = new OpenACCWorkerClause();
      current_clauses = new std::vector<OpenACCClause *>();
      current_clauses->push_back(new_clause);
      (*all_clauses)[ACCC_worker] = current_clauses;
    } else {
      new_clause = new OpenACCWorkerClause();
      current_clauses->push_back(new_clause);
    };
  }

  return new_clause;
};

void OpenACCWorkerClause::mergeClause(OpenACCDirective *directive,
                                      OpenACCClause *current_clause) {
  // Respect the global clause merging flag
  if (!OpenACCDirective::getClauseMerging()) {
    return;
  }

  std::vector<OpenACCClause *> *current_clauses =
      directive->getClauses(ACCC_worker);
  if (current_clauses->size() < 2) {
    return;
  }

  auto *incoming = static_cast<OpenACCWorkerClause *>(current_clause);
  for (auto it = current_clauses->begin(); it != current_clauses->end() - 1;
       ++it) {
    auto *existing = static_cast<OpenACCWorkerClause *>(*it);
    if (existing->getModifier() == incoming->getModifier() &&
        existing->getNumExpr().text == incoming->getNumExpr().text) {
      current_clauses->pop_back();
      directive->getClausesInOriginalOrder()->pop_back();
      delete incoming;
      break;
    }
  }
};

static OpenACCDeviceTypeKind parseDeviceTypeKind(const std::string &value) {
  std::string lower = value;
  std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
  if (lower == "host") {
    return ACCC_DEVICE_TYPE_host;
  }
  if (lower == "any") {
    return ACCC_DEVICE_TYPE_any;
  }
  if (lower == "multicore") {
    return ACCC_DEVICE_TYPE_multicore;
  }
  if (lower == "default") {
    return ACCC_DEVICE_TYPE_default;
  }
  return ACCC_DEVICE_TYPE_unknown;
}

void OpenACCDeviceTypeClause::addDeviceType(OpenACCDeviceTypeKind kind) {
  if (kind == ACCC_DEVICE_TYPE_unknown) {
    return;
  }
  for (const auto &existing : device_types) {
    if (existing == kind) {
      return;
    }
  }
  device_types.push_back(kind);
}

void OpenACCDeviceTypeClause::addDeviceTypeString(const std::string &value) {
  OpenACCDeviceTypeKind kind = parseDeviceTypeKind(value);
  if (kind == ACCC_DEVICE_TYPE_unknown) {
    if (std::find(unknown_types.begin(), unknown_types.end(), value) ==
        unknown_types.end()) {
      unknown_types.push_back(value);
    }
  } else {
    addDeviceType(kind);
  }
}

OpenACCClause *OpenACCDeviceTypeClause::addClause(OpenACCDirective *directive) {
  auto *new_clause = new OpenACCDeviceTypeClause();
  std::vector<OpenACCClause *> *current_clauses =
      directive->getClauses(ACCC_device_type);
  if (current_clauses->size() == 0) {
    current_clauses->push_back(new_clause);
  } else if (directive->getClauseMerging()) {
    delete new_clause;
    new_clause =
        dynamic_cast<OpenACCDeviceTypeClause *>(current_clauses->at(0));
    if (new_clause == nullptr) {
      new_clause = new OpenACCDeviceTypeClause();
      current_clauses->push_back(new_clause);
    }
  } else {
    current_clauses->push_back(new_clause);
  }
  return new_clause;
}

void OpenACCDeviceTypeClause::mergeClause(OpenACCDirective *directive,
                                          OpenACCClause *merge_clause) {
  if (!directive->getClauseMerging()) {
    return;
  }
  auto *other = dynamic_cast<OpenACCDeviceTypeClause *>(merge_clause);
  if (other == nullptr) {
    return;
  }
  for (auto k : other->device_types) {
    addDeviceType(k);
  }
  for (const auto &raw : other->unknown_types) {
    if (std::find(unknown_types.begin(), unknown_types.end(), raw) ==
        unknown_types.end()) {
      unknown_types.push_back(raw);
    }
  }
  auto *current_clauses = directive->getClauses(ACCC_device_type);
  if (!current_clauses->empty() &&
      current_clauses->back() == merge_clause &&
      current_clauses->size() > 1) {
    current_clauses->pop_back();
  }
  directive->getClausesInOriginalOrder()->pop_back();
  delete merge_clause;
}
