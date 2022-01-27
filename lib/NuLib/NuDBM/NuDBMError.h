/** @file NuDBMError.h
*   @brief DBM error code 
*   @note DBM base definiation declare
*                                         */

/* Error code from NuDBM */
/* -5000 ~ -5999 */

/* Error in DBM: -5000 ~ -5099 */
#define	NuDBMErr_DBMNotOpen					-5000
#define	NuDBMErr_DBMAlreadyOpen				-5001

/* Error in TABLE: -5100 ~ -5199 */
#define	NuDBMErr_TableNoPKey				-5100

/* Error in ROW: -5200 ~ -5299 */
#define	NuDBMErr_InvalidRowNum				-5200

/* Error in INDEX: -5300 ~ -5399 */
#define	NuDBMErr_IndexNumFull				-5300
#define	NuDBMErr_UniqueIndexDup				-5301
#define	NuDBMErr_UpdateFail					-5302

/* Error in RECORDSET: -5400 ~ -5499 */
#define	NuDBMErr_InvalidRecordSet			-5400
#define	NuDBMErr_ReqTranButNot				-5401
#define	NuDBMErr_AlreadyLastData			-5402
#define NuDBMErr_InvalidName				-5403

/* Error in RECOVERY: -5500 ~ -5599 */
#define	NuDBMErr_RecoveryOrigNotExist		-5500
#define	NuDBMErr_RecoveryUnknownAction		-5501
#define	NuDBMErr_RecoveryInvalidLength		-5502
#define	NuDBMErr_RecoveryWriteErr			-5503

