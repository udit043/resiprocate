#include <iostream>
#include <map>
#include <string>
#include <errno.h>
#include "rutil/Errdes.hxx"

#include <openssl/ssl.h>
#include <openssl/x509.h>


#ifdef _WIN32
#include <winsock.h>
#include <windows.h>
#endif

#define OSERROR 1
#define SSLERROR 2
#define X509ERROR 3

using namespace std;

map <int, string> ErrornoErrorMsg;
map <int, string> OpenSSLErrorMsg;
map <int, string> X509ErrorMsg;
#ifdef _WIN32
map <int, string> WinErrorMsg;
#endif


string NumericError::SearchErrorMsg(int Error, int ClassCode)
{
    string result;
    switch(ClassCode)
    {
        case OSERROR:
        {
            #ifdef _WIN32
                result = WinErrorMsg[Error];
            #elif __linux__
                result = ErrornoErrorMsg[Error];
            #endif
            break;
        }
        case SSLERROR:
        result = OpenSSLErrorMsg[Error];
        break;
        case X509ERROR:
        result = X509ErrorMsg[Error];
        break;
    }
    if(result.length() == 0)
        result = "Unknown error";
    return result;
};

void ErrnoError::CreateMappingErrorMsg()
{
    int ErrorCode[134] = 
    {
          EPERM,ENOENT,ESRCH,EINTR,EIO,ENXIO,E2BIG,ENOEXEC,EBADF,ECHILD,EAGAIN,ENOMEM,EACCES,EFAULT,
        ENOTBLK,EBUSY,EEXIST,EXDEV,ENODEV,ENOTDIR,EISDIR,EINVAL,ENFILE,EMFILE,ENOTTY,ETXTBSY,EFBIG,
        ENOSPC,ESPIPE,EROFS,EMLINK,EPIPE,EDOM,ERANGE,EDEADLK,ENAMETOOLONG,ENOLCK,ENOSYS,ENOTEMPTY,ELOOP,
        EWOULDBLOCK,ENOMSG,EIDRM,ECHRNG,EL2NSYNC,EL3HLT,EL3RST,ELNRNG,EUNATCH,ENOCSI,EL2HLT,EBADE,EBADR,
        EXFULL,ENOANO,EBADRQC,EBADSLT,EDEADLOCK,EBFONT,ENOSTR,ENODATA,ETIME,ENOSR,ENONET,ENOPKG,EREMOTE,
        ENOLINK,EADV,ESRMNT,ECOMM,EPROTO,EMULTIHOP,EDOTDOT,EBADMSG,EOVERFLOW,ENOTUNIQ,EBADFD,EREMCHG,ELIBACC,
        ELIBBAD,ELIBSCN,ELIBMAX,ELIBEXEC,EILSEQ,ERESTART,ESTRPIPE,EUSERS,ENOTSOCK,EDESTADDRREQ,EMSGSIZE,
        EPROTOTYPE,ENOPROTOOPT,EPROTONOSUPPORT,ESOCKTNOSUPPORT,EOPNOTSUPP,EPFNOSUPPORT,EAFNOSUPPORT,EADDRINUSE,
        EADDRNOTAVAIL,ENETDOWN,ENETUNREACH,ENETRESET,ECONNABORTED,ECONNRESET,ENOBUFS,EISCONN,ENOTCONN,ESHUTDOWN,
        ETOOMANYREFS,ETIMEDOUT,ECONNREFUSED,EHOSTDOWN,EHOSTUNREACH,EALREADY,EINPROGRESS,ESTALE,EUCLEAN,ENOTNAM,
        ENAVAIL,EISNAM,EREMOTEIO,EDQUOT,ENOMEDIUM,EMEDIUMTYPE,ECANCELED,ENOKEY,EKEYEXPIRED,EKEYREVOKED,EKEYREJECTED,
        EOWNERDEAD,ENOTRECOVERABLE,ERFKILL,EHWPOISON,
    };

    string ErrorString[134] = 
    {
        "EPERM (Operation not permitted) 1",
        "ENOENT (No such file or directory) 2",
        "ESRCH (No such process) 3",
        "EINTR (Interrupted system call) 4",
        "EIO (Input/output error) 5",
        "ENXIO (No such device or address) 6",
        "E2BIG (Argument list too long) 7",
        "ENOEXEC (Exec format error) 8",
        "EBADF (Bad file descriptor) 9",
        "ECHILD (No child processes) 10",
        "EAGAIN (Resource temporarily unavailable) 11",
        "ENOMEM (Cannot allocate memory) 12",
        "EACCES (Permission denied) 13",
        "EFAULT (Bad address) 14",
        "ENOTBLK (Block device required) 15", 
        "EBUSY (Device or resource busy) 16",
        "EEXIST (File exists) 17",
        "EXDEV (Invalid cross-device link) 18",
        "ENODEV (No such device) 19",
        "ENOTDIR (Not a directory) 20",
        "EISDIR (Is a directory) 21",
        "EINVAL (Invalid argument) 22",
        "ENFILE (Too many open files in system) 23",
        "EMFILE (Too many open files) 24",
        "ENOTTY (Inappropriate ioctl for device) 25",
        "ETXTBSY (Text file busy) 26",
        "EFBIG (File too large) 27",
        "ENOSPC (No space left on device) 28",
        "ESPIPE (Illegal seek) 29",
        "EROFS (Read-only file system) 30",
        "EMLINK (Too many links) 31",
        "EPIPE (Broken pipe) 32",
        "EDOM (Numerical argument out of domain) 33",
        "ERANGE (Numerical result out of range) 34",
        "EDEADLK (Resource deadlock avoided) 35",
        "ENAMETOOLONG (File name too long) 36",
        "ENOLCK (No locks available) 37",
        "ENOSYS (Function not implemented) 38",
        "ENOTEMPTY (Directory not empty) 39",
        "ELOOP (Too many levels of symbolic links) 40",
        "EWOULDBLOCK (Resource temporarily unavailable) 41",
        "ENOMSG (No message of desired type) 42",
        "EIDRM (Identifier removed) 43",
        "ECHRNG (Channel number out of range) 44",
        "EL2NSYNC (Level 2 not synchronized) 45",
        "EL3HLT (Level 3 halted) 46",
        "EL3RST (Level 3 reset) 47",
        "ELNRNG (Link number out of range) 48",
        "EUNATCH (Protocol driver not attached) 49",
        "ENOCSI (No CSI structure available) 50",
        "EL2HLT (Level 2 halted) 51",
        "EBADE (Invalid exchange) 52",
        "EBADR (Invalid request descriptor) 53",
        "EXFULL (Exchange full) 54",
        "ENOANO (No anode) 55",
        "EBADRQC (Invalid request code) 56",
        "EBADSLT (Invalid slot) 57",
        "EDEADLOCK (Resource deadlock avoided) 58",
        "EBFONT (Bad font file format) 59",
        "ENOSTR (Device not a stream) 60",
        "ENODATA (No data available) 61",
        "ETIME (Timer expired) 62",
        "ENOSR (Out of streams resources) 63",
        "ENONET (Machine is not on the network) 64",
        "ENOPKG (Package not installed) 65",
        "EREMOTE (Object is remote) 66",
        "ENOLINK (Link has been severed) 67",
        "EADV (Advertise error) 68",
        "ESRMNT (Srmount error) 69",
        "ECOMM (Communication error on send) 70",
        "EPROTO (Protocol error) 71",
        "EMULTIHOP (Multihop attempted) 72",
        "EDOTDOT (RFS specific error) 73",
        "EBADMSG (Bad message) 74",
        "EOVERFLOW (Value too large for defined data type) 75",
        "ENOTUNIQ (Name not unique on network) 76",
        "EBADFD (File descriptor in bad state) 77",
        "EREMCHG (Remote address changed) 78",
        "ELIBACC (Can not access a needed shared library) 79",
        "ELIBBAD (Accessing a corrupted shared library) 80",
        "ELIBSCN (.lib section in a.out corrupted) 81",
        "ELIBMAX (Attempting to link in too many shared libraries) 82",
        "ELIBEXEC (Cannot exec a shared library directly) 83",
        "EILSEQ (Invalid or incomplete multibyte or wide character) 84",
        "ERESTART (Interrupted system call should be restarted) 85",
        "ESTRPIPE (Streams pipe error) 86",
        "EUSERS (Too many users) 87",
        "ENOTSOCK (Socket operation on non-socket) 88",
        "EDESTADDRREQ (Destination address required) 89",
        "EMSGSIZE (Message too long) 90",
        "EPROTOTYPE (Protocol wrong type for socket) 91",
        "ENOPROTOOPT (Protocol not available) 92",
        "EPROTONOSUPPORT (Protocol not supported) 93",
        "ESOCKTNOSUPPORT (Socket type not supported) 94",
        "EOPNOTSUPP (Operation not supported) 95",
        "EPFNOSUPPORT (Protocol family not supported) 96",
        "EAFNOSUPPORT (Address family not supported by protocol) 97",
        "EADDRINUSE (Address already in use) 98",
        "EADDRNOTAVAIL (Cannot assign requested address) 99",
        "ENETDOWN (Network is down) 100",
        "ENETUNREACH (Network is unreachable) 101",
        "ENETRESET (Network dropped connection on reset) 102",
        "ECONNABORTED (Software caused connection abort) 103",
        "ECONNRESET (Connection reset by peer) 104",
        "ENOBUFS (No buffer space available) 105",
        "EISCONN (Transport endpoint is already connected) 106",
        "ENOTCONN (Transport endpoint is not connected) 107",
        "ESHUTDOWN (Cannot send after transport endpoint shutdown) 108",
        "ETOOMANYREFS (Too many references: cannot splice) 109",
        "ETIMEDOUT (Connection timed out) 110",
        "ECONNREFUSED (Connection refused) 111",
        "EHOSTDOWN (Host is down) 112",
        "EHOSTUNREACH (No route to host) 113",
        "EALREADY (Operation already in progress) 114",
        "EINPROGRESS (Operation now in progress) 115",
        "ESTALE (Stale file handle) 116",
        "EUCLEAN (Structure needs cleaning) 117",
        "ENOTNAM (Not a XENIX named type file) 118",
        "ENAVAIL (No XENIX semaphores available) 119",
        "EISNAM (Is a named type file) 120",
        "EREMOTEIO (Remote I/O error) 121",
        "EDQUOT (Disk quota exceeded) 122",
        "ENOMEDIUM (No medium found) 123",
        "EMEDIUMTYPE (Wrong medium type) 124",
        "ECANCELED (Operation canceled) 125",
        "ENOKEY (Required key not available) 126",
        "EKEYEXPIRED (Key has expired) 127",
        "EKEYREVOKED (Key has been revoked) 128",
        "EKEYREJECTED (Key was rejected by service) 129",
        "EOWNERDEAD (Owner died) 130",
        "ENOTRECOVERABLE (State not recoverable) 131",
        "ERFKILL (Operation not possible due to RF-kill) 132",
        "EHWPOISON (Memory page has hardware error) 133",
    };  

    for (int i = 0; i < 134; ++i)
    {

        ErrornoErrorMsg[ErrorCode[i]] = ErrorString[i];             // for linux 
    }

#ifdef _WIN32
{

    int WindowsErrorCode[95] =
    {
        WSA_INVALID_HANDLE,WSA_NOT_ENOUGH_MEMORY,WSA_INVALID_PARAMETER,WSA_OPERATION_ABORTED,
        WSA_IO_INCOMPLETE,SA_IO_PENDING,WSAEINTR,WSAEBADF,WSAEACCES,WSAEFAULT,
        WSAEINVAL,WSAEMFILE,WSAEWOULDBLOCK,WSAEINPROGRESS,WSAEALREADY,WSAENOTSOCK,WSAEDESTADDRREQ,
        WSAEMSGSIZE,WSAEPROTOTYPE,WSAENOPROTOOPT,WSAEPROTONOSUPPORT,WSAESOCKTNOSUPPORT,WSAEOPNOTSUPP,
        WSAEPFNOSUPPORT,WSAEAFNOSUPPORT,WSAEADDRINUSE,WSAEADDRNOTAVAIL,WSAENETDOWN,WSAENETUNREACH,
        WSAECONNABORTED,WSAECONNRESET,WSAENOBUFS,WSAEISCONN,WSAENOTCONN,WSAENETRESET,WSAESHUTDOWN,
        WSAETOOMANYREFS,WSAETIMEDOUT,WSAECONNREFUSED,WSAELOOP,WSAENAMETOOLONG,WSAEHOSTDOWN,WSAEHOSTUNREACH,
        WSAENOTEMPTY,WSAEPROCLIM,WSAEUSERS,WSAEDQUOT,WSAESTALE,WSAEREMOTE,WSASYSNOTREADY,WSAVERNOTSUPPORTED,
        WSANOTINITIALISED,WSAEDISCON,WSAENOMORE,WSAECANCELLED,WSAEINVALIDPROCTABLE,WSAEINVALIDPROVIDER,
        WSAEPROVIDERFAILEDINIT,WSASYSCALLFAILURE,WSASERVICE_NOT_FOUND,WSATYPE_NOT_FOUND,WSA_E_NO_MORE,
        WSA_E_CANCELLED,WSAEREFUSED,WSAHOST_NOT_FOUND,WSATRY_AGAIN,WSANO_RECOVERY,WSANO_DATA,WSA_QOS_RECEIVERS,
        WSA_QOS_SENDERS,WSA_QOS_NO_SENDERS,WSA_QOS_NO_RECEIVERS,WSA_QOS_REQUEST_CONFIRMED,WSA_QOS_ADMISSION_FAILURE,
        WSA_QOS_POLICY_FAILURE,WSA_QOS_BAD_STYLE,WSA_QOS_BAD_OBJECT,WSA_QOS_TRAFFIC_CTRL_ERROR,WSA_QOS_GENERIC_ERROR,
        WSA_QOS_ESERVICETYPE,WSA_QOS_EFLOWSPEC,WSA_QOS_EPROVSPECBUF,WSA_QOS_EFILTERSTYLE,WSA_QOS_EFILTERTYPE,
        WSA_QOS_EFILTERCOUNT,WSA_QOS_EOBJLENGTH,WSA_QOS_EFLOWCOUNT,WSA_QOS_EUNKOWNPSOBJ,WSA_QOS_EPOLICYOBJ,
        WSA_QOS_EFLOWDESC,WSA_QOS_EPSFLOWSPEC,WSA_QOS_EPSFILTERSPEC,WSA_QOS_ESDMODEOBJ,WSA_QOS_ESHAPERATEOBJ,
        WSA_QOS_RESERVED_PETYPE,
  };

    string WindowsErrorString[95] =
    {
        "WSA_INVALID_HANDLE 6 Specified event object handle is invalid.",
        "WSA_NOT_ENOUGH_MEMORY 8 Insufficient memory available.",
        "WSA_INVALID_PARAMETER 87 One or more parameters are invalid.",
        "WSA_OPERATION_ABORTED 995 Overlapped operation aborted.",
        "WSA_IO_INCOMPLETE 996 Overlapped I/O event object not in signaled state.",
        "SA_IO_PENDING 997 Overlapped operations will complete later.",
        "WSAEINTR 10004 Interrupted function call.",
        "WSAEBADF 10009 File handle is not valid.",
        "WSAEACCES 10013 Permission denied.",
        "WSAEFAULT 10014 Bad address.",
        "WSAEINVAL 10022 Invalid argument.",
        "WSAEMFILE 10024 Too many open files.",
        "WSAEWOULDBLOCK 10035 Resource temporarily unavailable.",
        "WSAEINPROGRESS 10036 Operation now in progress.",
        "WSAEALREADY 10037 Operation already in progress.",
        "WSAENOTSOCK 10038 Socket operation on nonsocket.",
        "WSAEDESTADDRREQ 10039 Destination address required.",
        "WSAEMSGSIZE 10040 Message too long.",
        "WSAEPROTOTYPE 10041 Protocol wrong type for socket.",
        "WSAENOPROTOOPT 10042 Bad protocol option.",
        "WSAEPROTONOSUPPORT 10043 Protocol not supported.",
        "WSAESOCKTNOSUPPORT 10044 Socket type not supported.",
        "WSAEOPNOTSUPP 10045 Operation not supported.",
        "WSAEPFNOSUPPORT 10046 Protocol family not supported.",
        "WSAEAFNOSUPPORT 10047 Address family not supported by protocol family.",
        "WSAEADDRINUSE 10048 Address already in use.",
        "WSAEADDRNOTAVAIL 10049 Cannot assign requested address.",
        "WSAENETDOWN 10050 Network is down.",
        "WSAENETUNREACH 10051 Network is unreachable.",
        "WSAENETRESET 10052 Network dropped connection on reset.",
        "WSAECONNABORTED 10053 Software caused connection abort.",
        "WSAECONNRESET 10054 Connection reset by peer.",
        "WSAENOBUFS 10055 No buffer space available.",
        "WSAEISCONN 10056 Socket is already connected.",
        "WSAENOTCONN 10057 Socket is not connected.",
        "WSAESHUTDOWN 10058 Cannot send after socket shutdown.",
        "WSAETOOMANYREFS 10059 Too many references.",
        "WSAETIMEDOUT 10060 Connection timed out.",
        "WSAECONNREFUSED 10061 Connection refused.",
        "WSAELOOP 10062 Cannot translate name.",
        "WSAENAMETOOLONG 10063 Name too long.",
        "WSAEHOSTDOWN 10064 Host is down.",
        "WSAEHOSTUNREACH 10065 No route to host.",
        "WSAENOTEMPTY 10066 Directory not empty.",
        "WSAEPROCLIM 10067 Too many processes.",
        "WSAEUSERS 10068 User quota exceeded.",
        "WSAEDQUOT 10069 Disk quota exceeded.",
        "WSAESTALE 10070 Stale file handle reference.",
        "WSAEREMOTE 10071 Item is remote.",
        "WSASYSNOTREADY 10091 Network subsystem is unavailable.",
        "WSAVERNOTSUPPORTED 10092 Winsock.dll version out of range.",
        "WSANOTINITIALISED 10093 Successful WSAStartup not yet performed.",
        "WSAEDISCON 10101 Graceful shutdown in progress.",
        "WSAENOMORE 10102 No more results.",
        "WSAECANCELLED 10103 Call has been canceled.",
        "WSAEINVALIDPROCTABLE 10104 Procedure call table is invalid.",
        "WSAEINVALIDPROVIDER 10105 Service provider is invalid.",
        "WSAEPROVIDERFAILEDINIT 10106 Service provider failed to initialize.",
        "WSASYSCALLFAILURE 10107 System call failure.",
        "WSASERVICE_NOT_FOUND 10108 Service not found.",
        "WSATYPE_NOT_FOUND 10109 Class type not found.",
        "WSA_E_NO_MORE 10110 No more results.",
        "WSA_E_CANCELLED 10111 Call was canceled.",
        "WSAEREFUSED 10112 Database query was refused.",
        "WSAHOST_NOT_FOUND 11001 Host not found.",
        "WSATRY_AGAIN 11002 Nonauthoritative host not found.",
        "WSANO_RECOVERY 11003 This is a nonrecoverable error.",
        "WSANO_DATA 11004 Valid name, no data record of requested type.",
        "WSA_QOS_RECEIVERS 11005 QoS receivers.",
        "WSA_QOS_SENDERS 11006 QoS senders.",
        "WSA_QOS_NO_SENDERS 11007 No QoS senders.",
        "WSA_QOS_NO_RECEIVERS 11008 QoS no receivers.",
        "WSA_QOS_REQUEST_CONFIRMED 11009 QoS request confirmed.",
        "WSA_QOS_ADMISSION_FAILURE 11010 QoS admission error.",
        "WSA_QOS_POLICY_FAILURE 11011 QoS policy failure.",
        "WSA_QOS_BAD_STYLE 11012 QoS bad style.",
        "WSA_QOS_BAD_OBJECT 11013 QoS bad object.",
        "WSA_QOS_TRAFFIC_CTRL_ERROR 11014 QoS traffic control error.",
        "WSA_QOS_GENERIC_ERROR 11015 QoS generic error.",
        "WSA_QOS_ESERVICETYPE 11016 QoS service type error.",
        "WSA_QOS_EFLOWSPEC 11017 QoS flowspec error.",
        "WSA_QOS_EPROVSPECBUF 11018 Invalid QoS provider buffer.",
        "WSA_QOS_EFILTERSTYLE 11019 Invalid QoS filter style.",
        "WSA_QOS_EFILTERTYPE 11020 Invalid QoS filter type.",
        "WSA_QOS_EFILTERCOUNT 11021 Incorrect QoS filter count.",
        "WSA_QOS_EOBJLENGTH 11022 Invalid QoS object length.",
        "WSA_QOS_EFLOWCOUNT 11023 Incorrect QoS flow count.",
        "WSA_QOS_EUNKOWNPSOBJ 11024 Unrecognized QoS object.",
        "WSA_QOS_EPOLICYOBJ 11025 Invalid QoS policy object.",
        "WSA_QOS_EFLOWDESC 11026 Invalid QoS flow descriptor.",
        "WSA_QOS_EPSFLOWSPEC 11027 Invalid QoS provider-specific flowspec.",
        "WSA_QOS_EPSFILTERSPEC 11028 Invalid QoS provider-specific filterspec.",
        "WSA_QOS_ESDMODEOBJ 11029 Invalid QoS shape discard mode object.",
        "WSA_QOS_ESHAPERATEOBJ 11030 Invalid QoS shaping rate object.",
        "WSA_QOS_RESERVED_PETYPE 11031 Reserved policy QoS element type.",            
    };

    for (int i = 0; i < 95; ++i)
    {
        WinErrorMsg[WindowsErrorCode[i]] = WindowsErrorString[i];   // for windows
    }

};

#endif

}
void OpenSSLError::CreateMappingErrorMsg()
{
    int OpenSSLCode[9] =
    {
        SSL_ERROR_NONE,
        SSL_ERROR_SSL,
        SSL_ERROR_WANT_READ,
        SSL_ERROR_WANT_WRITE,
        SSL_ERROR_WANT_X509_LOOKUP,
        SSL_ERROR_SYSCALL,
        SSL_ERROR_ZERO_RETURN,
        SSL_ERROR_WANT_CONNECT,
        SSL_ERROR_WANT_ACCEPT,
    };

    string OpenSSLString[9] =
    {
        "SSL_ERROR_NONE 0",
        "SSL_ERROR_SSL 1",
        "SSL_ERROR_WANT_READ 2",
        "SSL_ERROR_WANT_WRITE 3",
        "SSL_ERROR_WANT_X509_LOOKUP 4",
        "SSL_ERROR_SYSCALL 5",
        "SSL_ERROR_ZERO_RETURN 6",
        "SSL_ERROR_WANT_CONNECT 7",
        "SSL_ERROR_WANT_ACCEPT 8",
    };

    for (int i = 0; i < 9; ++i)
    {
        OpenSSLErrorMsg[OpenSSLCode[i]] = OpenSSLString[i];
    }

};

void X509Error::CreateMappingErrorMsg()
{
    int X509ErrorCode[54] =
    {
        X509_V_OK,
        X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT,
        X509_V_ERR_UNABLE_TO_GET_CRL,
        X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE,
        X509_V_ERR_UNABLE_TO_DECRYPT_CRL_SIGNATURE,
        X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY,
        X509_V_ERR_CERT_SIGNATURE_FAILURE,
        X509_V_ERR_CRL_SIGNATURE_FAILURE,
        X509_V_ERR_CERT_NOT_YET_VALID,
        X509_V_ERR_CERT_HAS_EXPIRED,
        X509_V_ERR_CRL_NOT_YET_VALID,
        X509_V_ERR_CRL_HAS_EXPIRED,
        X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD,
        X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD,
        X509_V_ERR_ERROR_IN_CRL_LAST_UPDATE_FIELD, 
        X509_V_ERR_ERROR_IN_CRL_NEXT_UPDATE_FIELD,
        X509_V_ERR_OUT_OF_MEM,
        X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT,
        X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN,
        X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY,
        X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE,
        X509_V_ERR_CERT_CHAIN_TOO_LONG,
        X509_V_ERR_CERT_REVOKED,
        X509_V_ERR_INVALID_CA,
        X509_V_ERR_PATH_LENGTH_EXCEEDED,
        X509_V_ERR_INVALID_PURPOSE,
        X509_V_ERR_CERT_UNTRUSTED,
        X509_V_ERR_CERT_REJECTED,
        X509_V_ERR_SUBJECT_ISSUER_MISMATCH,
        X509_V_ERR_AKID_SKID_MISMATCH,
        X509_V_ERR_AKID_ISSUER_SERIAL_MISMATCH,
        X509_V_ERR_KEYUSAGE_NO_CERTSIGN,
        X509_V_ERR_UNABLE_TO_GET_CRL_ISSUER,
        X509_V_ERR_UNHANDLED_CRITICAL_EXTENSION,
        X509_V_ERR_KEYUSAGE_NO_CRL_SIGN,
        X509_V_ERR_UNHANDLED_CRITICAL_CRL_EXTENSION,
        X509_V_ERR_INVALID_NON_CA,
        X509_V_ERR_PROXY_PATH_LENGTH_EXCEEDED,
        X509_V_ERR_KEYUSAGE_NO_DIGITAL_SIGNATURE,
        X509_V_ERR_PROXY_CERTIFICATES_NOT_ALLOWED,
        X509_V_ERR_INVALID_EXTENSION,
        X509_V_ERR_INVALID_POLICY_EXTENSION,
        X509_V_ERR_NO_EXPLICIT_POLICY,
        X509_V_ERR_DIFFERENT_CRL_SCOPE,
        X509_V_ERR_UNSUPPORTED_EXTENSION_FEATURE,
        X509_V_ERR_UNNESTED_RESOURCE,
        X509_V_ERR_PERMITTED_VIOLATION,
        X509_V_ERR_EXCLUDED_VIOLATION,
        X509_V_ERR_SUBTREE_MINMAX,
        X509_V_ERR_APPLICATION_VERIFICATION,
        X509_V_ERR_UNSUPPORTED_CONSTRAINT_TYPE,
        X509_V_ERR_UNSUPPORTED_CONSTRAINT_SYNTAX,
        X509_V_ERR_UNSUPPORTED_NAME_SYNTAX,
        X509_V_ERR_CRL_PATH_VALIDATION_ERROR,
    };

    string X509ErrorString[54] = 
    {
        "X509_V_OK 0",
        "X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT 2",
        "X509_V_ERR_UNABLE_TO_GET_CRL 3",
        "X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE 4",
        "X509_V_ERR_UNABLE_TO_DECRYPT_CRL_SIGNATURE 5",
        "X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY 6",
        "X509_V_ERR_CERT_SIGNATURE_FAILURE 7",
        "X509_V_ERR_CRL_SIGNATURE_FAILURE 8",
        "X509_V_ERR_CERT_NOT_YET_VALID 9",
        "X509_V_ERR_CERT_HAS_EXPIRED 10",
        "X509_V_ERR_CRL_NOT_YET_VALID 11",
        "X509_V_ERR_CRL_HAS_EXPIRED 12",
        "X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD 13",
        "X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD 14",
        "X509_V_ERR_ERROR_IN_CRL_LAST_UPDATE_FIELD 15", 
        "X509_V_ERR_ERROR_IN_CRL_NEXT_UPDATE_FIELD 16",
        "X509_V_ERR_OUT_OF_MEM 17",
        "X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT 18",
        "X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN 19",
        "X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY 20",
        "X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE 21",
        "X509_V_ERR_CERT_CHAIN_TOO_LONG 22",
        "X509_V_ERR_CERT_REVOKED 23",
        "X509_V_ERR_INVALID_CA 24",
        "X509_V_ERR_PATH_LENGTH_EXCEEDED 25",
        "X509_V_ERR_INVALID_PURPOSE 26",
        "X509_V_ERR_CERT_UNTRUSTED 27",
        "X509_V_ERR_CERT_REJECTED 28",
        "X509_V_ERR_SUBJECT_ISSUER_MISMATCH 29",
        "X509_V_ERR_AKID_SKID_MISMATCH 30",
        "X509_V_ERR_AKID_ISSUER_SERIAL_MISMATCH 31",
        "X509_V_ERR_KEYUSAGE_NO_CERTSIGN 32",
        "X509_V_ERR_UNABLE_TO_GET_CRL_ISSUER 33",
        "X509_V_ERR_UNHANDLED_CRITICAL_EXTENSION 34",
        "X509_V_ERR_KEYUSAGE_NO_CRL_SIGN 35",
        "X509_V_ERR_UNHANDLED_CRITICAL_CRL_EXTENSION 36",
        "X509_V_ERR_INVALID_NON_CA 37",
        "X509_V_ERR_PROXY_PATH_LENGTH_EXCEEDED 38",
        "X509_V_ERR_KEYUSAGE_NO_DIGITAL_SIGNATURE 39",
        "X509_V_ERR_PROXY_CERTIFICATES_NOT_ALLOWED 40",
        "X509_V_ERR_INVALID_EXTENSION 41",
        "X509_V_ERR_INVALID_POLICY_EXTENSION 42",
        "X509_V_ERR_NO_EXPLICIT_POLICY 43",
        "X509_V_ERR_DIFFERENT_CRL_SCOPE 44",
        "X509_V_ERR_UNSUPPORTED_EXTENSION_FEATURE 45",
        "X509_V_ERR_UNNESTED_RESOURCE 46",
        "X509_V_ERR_PERMITTED_VIOLATION 47",
        "X509_V_ERR_EXCLUDED_VIOLATION 48",
        "X509_V_ERR_SUBTREE_MINMAX 49",
        "X509_V_ERR_APPLICATION_VERIFICATION 50",
        "X509_V_ERR_UNSUPPORTED_CONSTRAINT_TYPE 51",
        "X509_V_ERR_UNSUPPORTED_CONSTRAINT_SYNTAX 52",
        "X509_V_ERR_UNSUPPORTED_NAME_SYNTAX 53",
        "X509_V_ERR_CRL_PATH_VALIDATION_ERROR 54",
    };

    for (int i = 0; i < 54; ++i)
    {
       X509ErrorMsg[X509ErrorCode[i]] = X509ErrorString[i];
    }
};

NumericError search;

#ifdef _WIN32
ErrnoError WinObj;
#endif

ErrnoError ErrornoObj;
X509Error X509Obj;
OpenSSLError OpenSSLObj;
