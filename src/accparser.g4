parser grammar accparser;

options { tokenVocab = acclexer; }

directive
  : ATOMIC clause_sequence? EOF
  | CACHE payload EOF
  | DATA clause_sequence? EOF
  | DECLARE clause_sequence? EOF
  | END end_kind EOF
  | ENTER DATA clause_sequence? EOF
  | EXIT DATA clause_sequence? EOF
  | HOST_DATA clause_sequence? EOF
  | INIT clause_sequence? EOF
  | KERNELS LOOP clause_sequence? EOF
  | KERNELS clause_sequence? EOF
  | LOOP clause_sequence? EOF
  | PARALLEL LOOP clause_sequence? EOF
  | PARALLEL clause_sequence? EOF
  | ROUTINE payload? clause_sequence? EOF
  | SERIAL LOOP clause_sequence? EOF
  | SERIAL clause_sequence? EOF
  | SET clause_sequence? EOF
  | SHUTDOWN clause_sequence? EOF
  | UPDATE clause_sequence? EOF
  | WAIT payload? clause_sequence? EOF
  ;

end_kind
  : ATOMIC
  | DATA
  | HOST_DATA
  | KERNELS LOOP?
  | PARALLEL LOOP?
  | SERIAL LOOP?
  | LOOP
  | word
  ;

clause_sequence
  : clause (COMMA? clause)*
  ;

clause
  : word payload?
  ;

payload
  : LEFT_PAREN PAYLOAD RIGHT_PAREN
  ;

word
  : ATOMIC
  | CACHE
  | DATA
  | DECLARE
  | END
  | ENTER
  | EXIT
  | HOST_DATA
  | INIT
  | KERNELS
  | LOOP
  | PARALLEL
  | ROUTINE
  | SERIAL
  | SET
  | SHUTDOWN
  | UPDATE
  | WAIT
  | ASYNC
  | ATTACH
  | AUTO
  | BIND
  | CAPTURE
  | COLLAPSE
  | COPY
  | COPYIN
  | COPYOUT
  | CREATE
  | DEFAULT_ASYNC
  | DEFAULT
  | DELETE
  | DETACH
  | DEVICE
  | DEVICE_NUM
  | DEVICE_RESIDENT
  | DEVICE_TYPE
  | DEVICEPTR
  | FINALIZE
  | FIRSTPRIVATE
  | GANG
  | HOST
  | IF
  | IF_PRESENT
  | INDEPENDENT
  | LINK
  | NO_CREATE
  | NOHOST
  | NUM_GANGS
  | NUM_WORKERS
  | PCOPY
  | PCOPYIN
  | PCOPYOUT
  | PCREATE
  | PRESENT_OR_COPY
  | PRESENT_OR_COPYIN
  | PRESENT_OR_COPYOUT
  | PRESENT_OR_CREATE
  | PRESENT
  | PRIVATE
  | READ
  | REDUCTION
  | SELF
  | SEQ
  | TILE
  | USE_DEVICE
  | VECTOR
  | VECTOR_LENGTH
  | WORKER
  | WRITE
  | DTYPE
  | IDENTIFIER
  ;
