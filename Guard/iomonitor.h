#ifndef __IOMONITOR__ 
#define __IOMONITOR__
#include "wdm.h"
//#include "ntddk.h"
#include "stdarg.h"
#include "stdio.h"

//#include "SMBuffer.h"
#include "SMTypeDefine.h"
#define LOGTOFILE
#define  LOGEE
//#define IOHOOKMONITOR
#define DRIVERTAG 'OMOI'


//------Buffer define--------
#define CountBuffer 10
extern const UCHAR COUNTFILEJBUFFER ;
#define BlockSize 0x1000
#define CountBlock 3 
//--------------------------
#define MAX_PATH 260

#define ObjectHashTableLen 0x100 
#define OWNMAXPATHLEN    sizeof(WCHAR) * (MAX_PATH+1)
#define NameInformationBefferSize sizeof(ULONG)+ OWNMAXPATHLEN // 260 = MaxPath + 1 NULL

#ifndef POOL_RAISE_IF_ALLOCATION_FAILURE
  #define POOL_RAISE_IF_ALLOCATION_FAILURE 16
#endif
 
#ifndef MIN
#define MIN(a, b)	(((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#endif


typedef
NTSTATUS
DRIVER_DISPATCH_BYFILTER (
    __in struct _DEVICE_OBJECT *DeviceObject,
    __in struct _IRP *Irp , 
	__in struct _IO_STACK_LOCATION *IrpStack 
    );

extern FAST_IO_DISPATCH    FastIOHook ;
extern KEVENT Hevent;
typedef DRIVER_DISPATCH_BYFILTER *PDRIVER_DISPATCH_BYFILTER;

typedef enum _DEVICEENUM { GUIDEVICE = 1, FILTERDRIVER } DEVICEENUM ;
typedef enum _DEVICEMODEL { FileSystemDevice , VolumeMounted , Volume  } DEVICEMODEL;



typedef struct _DEVICE_EXTENSION
{
	DEVICEENUM TYPE ;
	PDEVICE_OBJECT	LowerDevice ; 
	PDEVICE_OBJECT	FileSystem ; 
	BOOLEAN HOOKED ;
	WCHAR            DosDeviceName[MAX_PATH] ;
	//UNICODE_STRING  DosDeviceName ;
	DEVICEMODEL MODEL ;

}DEVICE_EXTENSION_AV , *PDEVICE_EXTENSION_AV ;

extern SINGLE_LIST_ENTRY SingleHead  ;

typedef struct  _FileHandle_
{
	PDEVICE_OBJECT	FileSysDevice ;
	PFILE_OBJECT	FileObject;
	WCHAR*			wsDosDeviceName;
	WCHAR*			wsFileName;
}FileHandle , *PFileHandle;

typedef struct __FileObjectEntry 
{
	PFILE_OBJECT    FileObject;
	LIST_ENTRY		Entry ; 
}FileObjectEntry , *PFileObjectEntry;

typedef struct __HashTableRow
{
	LIST_ENTRY  FileObjectHashTeble ;
	KSPIN_LOCK  ObjectListSpinLock ;
}HashTableRow , *PHashTableRow;



typedef struct _SMBuffer        SMBuffer ,		*PSMBuffer ;
typedef struct _DatFileParser	DatFileParser , *PDatFileParser ;
typedef struct _tagLoadDatFile	tagLoadDatFile ,*PtagLoadDatFile ;
typedef struct _SbScaner		SbScaner ,		*PSbScaner ;
typedef struct _VirusDatFile	SMVirusDatFile, *PSMVirusDatFile;
typedef struct _SMVirtualMachineStack SMVirtualMachineStack, *PSMVirtualMachineStack ;
typedef struct __GlobalVariable
{
	PDRIVER_OBJECT			SMAV_Driver ;
	FAST_MUTEX				gbAttachMutex ;
	FAST_MUTEX				gbAllocLookAside ; 
	NPAGED_LOOKASIDE_LIST	NPLLHashTableEntry ;
	//NPAGED_LOOKASIDE_LIST NPLLPath ;
	PHashTableRow			FileObjectHashTeble ; 
	PSMBuffer				BufferPool[CountBuffer] ;
	LONG					StartBufferAccsess ;
	KSEMAPHORE				BufferSemaphore ;
	FAST_MUTEX				BufferPointerLock ;
	PKEVENT					Hevent ;
	PSbScaner				SbScanerPool[CountBuffer] ;
	PDatFileParser			DatFileParserPool[CountBuffer];
	PSMVirtualMachineStack	parosVMStack[CountBuffer];
	PtagLoadDatFile			LoadDatFile; 
	WCHAR					*VirusString ;
	BOOLEAN					GuiConnect ;
	PSMVirusDatFile			pocSMVirusDatFile;
	ULONG					u32DemandConfig;
} GlobalVariable , *PGlobalVariable ;

extern GlobalVariable GV ;


NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath );
DRIVER_UNLOAD     DriverUnload ;
NTSTATUS DispatchRoutine  (IN PDEVICE_OBJECT DeviceObject,IN PIRP Irp);
NTSTATUS OwnDeviceDispatch (IN PDEVICE_OBJECT DeviceObject,IN PIRP Irp);
NTSTATUS FilterDeviceDipatch (IN PDEVICE_OBJECT DeviceObject,IN PIRP Irp);
NTSTATUS DispatchControl (IN PDEVICE_OBJECT DeviceObject,IN PIRP Irp);
NTSTATUS CreateFile ( WCHAR* FileName ,HANDLE *hfile ); 

VOID FsChangeNotify ( IN PDEVICE_OBJECT DeviceObject,	IN BOOLEAN FsActive ) ;

 
PDRIVER_DISPATCH_BYFILTER	OwnMajorFunction [IRP_MJ_MAXIMUM_FUNCTION + 1];
PDRIVER_DISPATCH_BYFILTER	FilterMajorFunction[IRP_MJ_MAXIMUM_FUNCTION + 1];

__inline void * SMAlloc (size_t _Size)
{
	//return malloc(_Size);
	return ExAllocatePoolWithTag( PagedPool ,_Size , DRIVERTAG ) ;
}
__inline  void SMFree(void * Buf)
{
	ExFreePool(Buf) ;
}

#endif __IOMONITOR__