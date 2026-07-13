lexer grammar acclexer;

// The directive envelope and balanced host-language fragments are scanned by
// OpenACCASTConstructor.cpp. ANTLR sees only OpenACC structural tokens and
// opaque payload identifiers. This keeps base-language punctuation out of the
// OpenACC grammar while preserving all punctuation needed by the validator.

PAYLOAD : '__acc_payload_' [0-9]+;

LEFT_PAREN : '(';
RIGHT_PAREN : ')';
COMMA : ',';

ATOMIC : 'atomic';
CACHE : 'cache';
DATA : 'data';
DECLARE : 'declare';
END : 'end';
ENTER : 'enter';
EXIT : 'exit';
HOST_DATA : 'host_data';
INIT : 'init';
KERNELS : 'kernels';
LOOP : 'loop';
PARALLEL : 'parallel';
ROUTINE : 'routine';
SERIAL : 'serial';
SET : 'set';
SHUTDOWN : 'shutdown';
UPDATE : 'update';
WAIT : 'wait';

ASYNC : 'async';
ATTACH : 'attach';
AUTO : 'auto';
BIND : 'bind';
CAPTURE : 'capture';
COLLAPSE : 'collapse';
COPY : 'copy';
COPYIN : 'copyin';
COPYOUT : 'copyout';
CREATE : 'create';
DEFAULT_ASYNC : 'default_async';
DEFAULT : 'default';
DELETE : 'delete';
DETACH : 'detach';
DEVICE_NUM : 'device_num';
DEVICE_RESIDENT : 'device_resident';
DEVICE_TYPE : 'device_type';
DEVICEPTR : 'deviceptr';
DEVICE : 'device';
FINALIZE : 'finalize';
FIRSTPRIVATE : 'firstprivate';
GANG : 'gang';
HOST : 'host';
IF_PRESENT : 'if_present';
IF : 'if';
INDEPENDENT : 'independent';
LINK : 'link';
NO_CREATE : 'no_create';
NOHOST : 'nohost';
NUM_GANGS : 'num_gangs';
NUM_WORKERS : 'num_workers';
PCOPY : 'pcopy';
PCOPYIN : 'pcopyin';
PCOPYOUT : 'pcopyout';
PCREATE : 'pcreate';
PRESENT_OR_COPY : 'present_or_copy';
PRESENT_OR_COPYIN : 'present_or_copyin';
PRESENT_OR_COPYOUT : 'present_or_copyout';
PRESENT_OR_CREATE : 'present_or_create';
PRESENT : 'present';
PRIVATE : 'private';
READ : 'read';
REDUCTION : 'reduction';
SELF : 'self';
SEQ : 'seq';
TILE : 'tile';
USE_DEVICE : 'use_device';
VECTOR_LENGTH : 'vector_length';
VECTOR : 'vector';
WORKER : 'worker';
WRITE : 'write';
DTYPE : 'dtype';

IDENTIFIER : [A-Za-z_] [A-Za-z0-9_]*;

WS : [ \t\r\n]+ -> skip;

INVALID : .;
