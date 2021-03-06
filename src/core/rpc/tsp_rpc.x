/* -*- idl -*-

$Header: /sources/tsp/tsp/src/core/rpc/tsp_rpc.x,v 1.43 2008/02/05 18:54:11 rhdv Exp $

-----------------------------------------------------------------------

Project   : TSP

Component : Consumer / Provider

-----------------------------------------------------------------------

Purpose   : 

-----------------------------------------------------------------------
 */

/**
 * @defgroup TSP_AbstractAPI The TSP Abstract API
 * The TSP API module is the one which define the TSP global API
 * for both consumer and provider. For now this is done through
 * the RPC IDL of the @ref TSP_RPCRequestHandlerLib, 
 * but this should be replaced by a request handler neutral
 * API which will be called by each request handler.
 * The TSP Abstract API directly comes from TSP Specifications,
 * it contains the definition of:
 * <ul>
 *    <li> Some constants </li>
 *    <li> The TSP asynchronous requests with associated answer 
 *         <ul>
 *            <li> TSP Request Open @ref TSP_request_open_t </li>
 *            <li> TSP Answer Open @ref TSP_answer_open_t </li>
 *            <li> TSP Request Close @ref TSP_request_close_t</li>
 *            <li> TSP Request Information @ref TSP_request_information_t </li>
 *            <li> TSP Request Filtered Information @ref TSP_request_information_t </li>
 *            <li> TSP Request Extended Information @ref TSP_request_extended_information_t </li>
 *            <li> TSP Answer Sample @ref TSP_answer_sample_t </li>
 *            <li> TSP Request Sample @ref TSP_request_sample_t </li>
 *            <li> TSP Request Sample Initialisation @ref TSP_request_sample_init_t</li>
 *            <li> TSP Answer Sample Initialisation @ref TSP_answer_sample_init_t</li>
 *            <li> TSP Request Sample Destroy  @ref TSP_request_sample_destroy_t </li>
 *            <li> TSP Answer Sample Destroy @ref TSP_answer_sample_destroy_t </li>
 *            <li> TSP Request Async Sample Read </li>
 *            <li> TSP Request Async Sample Write </li>
 *            <li> TSP Request Feature </li>
 *            <li> TSP Answer Feature </li>
 *            <li> TSP Exec Feature </li>
 *         </ul>
 *    </li>
 * </ul>
 * @todo Use TSP_status_t ENUM where possible
 * @todo Add channel id to async_read/write
 * @ingroup TSP_CoreLib
 * @{
 */

/** Status for all functions */
enum TSP_status_t 
{
	/** OK */
	TSP_STATUS_OK=0,

	/** NOK but not an error */
	TSP_STATUS_NOK=1,

	/** 
	 * Provider unreachable.
	 * The provider may be stopped or dead.
	 */
	TSP_STATUS_ERROR_PROVIDER_UNREACHABLE=2,

	/** Fatal error */
	TSP_STATUS_ERROR_UNKNOWN=3,

	/** 
	 * Means that a detailed error string is provided by the function,
	 * and the details of the error are in this string
         */
	TSP_STATUS_ERROR_SEE_STRING=4,	

	/** 
         * The requested version for the protocol does not match
         */
	TSP_STATUS_ERROR_VERSION=5,

	/**
         * Error with the symbols (asked or returned)
         */
	TSP_STATUS_ERROR_SYMBOLS=6,

	/**
	 * The requested symbols filter is ill-formed
	 */
	TSP_STATUS_ERROR_SYMBOL_FILTER=7,

	/**
         * The requested feature is not supported (by this particular consumer/provider)
         */
	TSP_STATUS_ERROR_NOT_SUPPORTED=8,

	/**	
         * The requested feature is not implemented
         */
	TSP_STATUS_ERROR_NOT_IMPLEMENTED=9,

	/**
         * The requested PGI (provider global index is unknown)
         */
	TSP_STATUS_ERROR_PGI_UNKNOWN=10,

	/**
         * Asynchronous read is not allowed.
         */
 	TSP_STATUS_ERROR_ASYNC_READ_NOT_ALLOWED=11,

	/**
         * Asynchronous write is not allowed.
         */
 	TSP_STATUS_ERROR_ASYNC_WRITE_NOT_ALLOWED=12,       

	/**
         * Asynchronous read is not supported.
         */
	TSP_STATUS_ERROR_ASYNC_READ_NOT_SUPPORTED=13,	

	/**
         * Asynchronous write is not supported.
         */
 	TSP_STATUS_ERROR_ASYNC_WRITE_NOT_SUPPORTED=14,     	
	/**
	 * Memory Allocation error
	 */
	TSP_STATUS_ERROR_MEMORY_ALLOCATION=15,
	/**
	 * Provided channel Id is invalid
	 */
	TSP_STATUS_ERROR_INVALID_CHANNEL_ID=16,
	/**
	 * Cannot instantiate a new GLU
	 */
	TSP_STATUS_ERROR_NO_MORE_GLU=17,
	/**
	 * Cannot create new TSP Session
	 * Maximum reached.
	 */
	TSP_STATUS_ERROR_NO_MORE_SESSION=18,
	/**
	 * Cannot start the GLU 
	 */
	TSP_STATUS_ERROR_GLU_START=19,
	/**
	 * Cannot initialize GLU
	 */
	TSP_STATUS_ERROR_GLU_INITIALIZE=20,
	/**
	 * The logical ordering of TSP Request was
	 * not respected. E.g. Request Sample Initialize
	 * before Request Sample.
	 */
	TSP_STATUS_ERROR_BAD_REQUEST_ORDER=21,
	/**
	 * Cannot instantiate new Datapool.
	 */
	TSP_STATUS_ERROR_DATAPOOL_INSTANTIATE=22,
	/**
	 * Cannot create new thread.
	 */
	TSP_STATUS_ERROR_THREAD_CREATE=23,
	
	TSP_STATUS_ERROR_NOT_INITIALIZED=24,
	/**
	 * The TSP Request content is invalid.
	 */
	TSP_STATUS_ERROR_INVALID_REQUEST=25,
	/**
	 * The TSP Request sample is empty.
	 * i.e. no symbol requested.
	 */
	TSP_STATUS_ERROR_EMPTY_REQUEST_SAMPLE=26,
	/**
	 * Value greater than TSP_STATUS_ERROR_CUSTOM_BEGIN
	 * may be used by custom TSP providers of consumers
	 * without core TSP_STATUS_ERROR_ collision.
	 */
	TSP_STATUS_ERROR_CUSTOM_BEGIN=16384
};

/**
 * The different status of a TSP session.
 * A TSP session is a link between one consumer
 * and one provider.
 */
enum TSP_session_state_t {
  /** This session is probably not initialized */
  TSP_SESSION_STATE_UNKNOWN=0,
  /** This session has accepted a valid request open */
  TSP_SESSION_STATE_OPENED=1,
  /** This session has received a valid request close */
  TSP_SESSION_STATE_CLOSED=2,
  /** This session has accepted a valid request sample */
  TSP_SESSION_STATE_REQUEST_SAMPLE_OK=3,
  /** 
   * This session has accepted a valid request sample init
   * but sampling did not began (yet) because consumer
   * is not connected yet. 
   */
  TSP_SESSION_STATE_REQUEST_SAMPLE_INIT_OK=4,
  /**
   * Consumer connected and provdier is sending sampling
   * for this session.
   */
  TSP_SESSION_STATE_SAMPLING=5,
  /** 
   * This session has accepted a valid request sample destroy.
   */
  TSP_SESSION_STATE_REQUEST_SAMPLE_DESTROY_OK=6,
  /**
   * Was in sampling state but link goes to broken.
   * May be a network failure or either party (provider or consumer)
   * crashed or terminate itself without sending
   * request sample destroy and/or request close.
   */
  TSP_SESSION_STATE_BROKEN_LINK=7,
  TSP_SESSION_STATE_CLOSE_ON_EOF=8
};
			 

/** TSP Symbols basic types */
enum TSP_datatype_t 
{
  /**
   *  Unknown is used when type is unknown.
   *  Consumer may use it in its request sample when he 
   *  does not know the exact type of the symbol (he may only
   *  known its name).
   *  Provider may use it in answer sample when requested type
   *  does not match the one found on provider side.
   */
  TSP_TYPE_UNKNOWN=0,
  TSP_TYPE_DOUBLE=1,   /*!< An IEEE double precision floating point  */
  TSP_TYPE_FLOAT=2,      /*!< An IEEE simple precision floating point  */
  TSP_TYPE_INT8=3,       /*!< An 8bit signed integer                   */
  TSP_TYPE_INT16=4,      /*!< A 16bit signed integer                   */
  TSP_TYPE_INT32=5,      /*!< A 32bit signed integer                   */
  TSP_TYPE_INT64=6,      /*!< A 64bit signed integer                   */
  TSP_TYPE_UINT8=7,      /*!< An 8bit unsigned integer                 */
  TSP_TYPE_UINT16=8,     /*!< A 16bit unsigned integer                 */
  TSP_TYPE_UINT32=9,     /*!< A 32bit unsigned integer                 */
  TSP_TYPE_UINT64=10,     /*!< A 64bit unsigned integer                 */
  TSP_TYPE_CHAR=11,       /*!< An 8bit signed character                 */
  TSP_TYPE_UCHAR=12,      /*!< An 8bit unsigned character               */
  /**
   * Any type of 8bit size.
   * For this type no encoding will be done
   * before sending it and no decode
   * will be done on receive.
   * Beware endianity problem when using this.
   */
  TSP_TYPE_RAW=13,      
  TSP_TYPE_LAST=14       
};


/** 
 * The TSP answer open is the answer
 * from a TSP request open.
 * @see TSP_request_open_t
 */
struct TSP_answer_open_t
{
        /** The version of TSP protocol */
        int version_id;            
        /** The channel identifier used to send forthcoming request,
	    which is a session identifier stored by the
	    TSP provider in order to handle provider-side
	    consumer state or data */
        unsigned int channel_id;   
        /** The provider status */
        TSP_status_t status;  
        /** A string describing the provider status */
	string status_str<>;

};


typedef string   TSP_argv_item_t<>;
typedef TSP_argv_item_t TSP_argv_t<>; 

/**  
 * TSP request open.
 * The TSP request open is the first TSP request
 * a TSP consumer may send a TSP provider 
 * to obtain a (unique) TSP channel identifier
 * in order to be able to send other TSP request.
 * @see TSP_answer_open_t
 * @see TSP_request_close_t
 */
struct TSP_request_open_t
{
 	/** The version of the TSP protocol */	
	int version_id;       
/*	string stream_init<>;
	int use_stream_init;*/
        /** The arguments which may be transmitted from
            consumer-side to provider-side in order
            to transmit initialization option  */
	TSP_argv_t argv;	
};

/**
 * TSP Request close.
 * This request may be sent for ending a TSP
 * session. All session related information on
 * provider side will be dropped.
 * The channel id may not be used any more.
 * @see TSP_request_open_t
 */
struct TSP_request_close_t
{
  /** TSP Protocol Version */
  int version_id;          
  /** TSP Asynchronous Channel Id  */
  unsigned int channel_id; 
};

/**
 * TSP Request Informations.
 * Ask TSP provider to send informations
 * about symbols.
 */
struct TSP_request_information_t
{
  /** TSP Protocol Version */
  int version_id;
  /** TSP Asynchronous Channel Id  */
  unsigned int channel_id;
};

/**
 * TSP Request feature.
 * Test if provider is implementing the features
 * specified by the provided feature words.
 * This is an extension point for specific 
 * TSP usage.
 */
struct TSP_request_feature_t
{
  /** TSP Protocol Version */
  int version_id;
  /** TSP Asynchronous Channel Id  */
  unsigned int channel_id;
  /** TSP Feature Word  */
  unsigned int feature_words[4];
};

/**
 * TSP Answer feature.
 * This is the answer to a TSP Request feature.
 */
struct TSP_answer_feature_t
{
  /** TSP Protocol Version */
  int version_id;
  /** TSP Asynchronous Channel Id  */
  unsigned int channel_id;
  /** TSP Feature Word  */
  unsigned int feature_words[4];
  
  int int_value;
  double double_value;
  string string_value<>;
  TSP_status_t status;		
};

/**
 * TSP Exec Feature.
 * After sending a TSP Request Feature a consumer
 * may want to 'exec' this feature.
 */
struct TSP_exec_feature_t
{
  /** TSP Protocol Version */
  int version_id;
  /** TSP Asynchronous Channel Id  */
  unsigned int channel_id;
  unsigned int feature_words[4];
  int int_value;
  double double_value;
  string string_value<>;
};

/**
 * TSP Sample Information.
 * A TSP Symbol is entirely defined by this
 * structure.
 */
struct TSP_sample_symbol_info_t
{
  /** TSP Symbol name */
  string name<>;
  /** 
   * The provider global index (PGI).
   * This a provider-side index which uniquely
   * identify a symbol. Each symbol provided by
   * a TSP provider should have a unique PGI assigned
   * by the provider.
   */
  int provider_global_index;
  /**
   * The TSP Group Index to which the symbol belongs to.
   * TSP Groups are computed by the provider
   * upon a TSP request sample.
   */
  int provider_group_index;
  /**
   * The rank of the symbol in its TSP Group.
   */
  int provider_group_rank;
  /**
   * The type of the TSP symbol.
   */
  TSP_datatype_t type;
  /**
   * The dimension of the symbol (array size).
   * <ul>
   *   <li> 1 for scalar </li> 
   *   <li> more than 1 for arrays. </li>
   * </ul>
   */
  unsigned int dimension;
  /**
   * The offset of the first requested element
   * if the symbol is an array.
   * Only used if dimension > 1.
   */
  unsigned int offset;
  /**
   * The number of requested element beginning at offset.
   * 0 means all element requested beginning at offset.
   * Only used if dimension > 1.
   */
  unsigned int nelem;
  /**
   * The period at which the symbol will be provided.
   */
  int period;
  /**
   * The phase at which the symbol will be provided.
   */
  int phase; 
};

/** List of @ref TSP_sample_symbol_info_t */
typedef TSP_sample_symbol_info_t TSP_sample_symbol_info_list_t<>;

/**
 * The TSP extended information structure
 * which is a simple pair of key/value
 */
struct TSP_extended_info_t {
  /** Extended info key */
  string key<>;
  /** Extended info value */
  string value<>;
};

/** List of @ref TSP_extended_info_t */
typedef TSP_extended_info_t TSP_extended_info_list_t<>;

/**
 * TSP Sample Extended Information.
 * A TSP Symbol may have extended information
 * which are not defined in @ref TSP_sample_symbol_info_t.
 * Those informations are defined by a list of
 * @ref TSP_extended_info_t.
 */
struct TSP_sample_symbol_extended_info_t
{ 
  /** 
   * The provider global index (PGI).
   * This a provider-side index which uniquely
   * identify a symbol. Each symbol provided by
   * a TSP provider should have a unique PGI assigned
   * by the provider.
   */
  int provider_global_index;
  /**
   * The extended information list.
   */
  TSP_extended_info_list_t info;
};

/** List of @ref TSP_sample_symbol_extended_info_t */
typedef TSP_sample_symbol_extended_info_t TSP_sample_symbol_extended_info_list_t<>;

/**
 * TSP Request Extended Informations.
 * Ask TSP provider to send extended informations
 * about TSP symbols whose PGI is given on the
 * pgi list, the @ref TSP_answer_feature_t will contains
 * the requested extended informations.
 */
struct TSP_request_extended_information_t
{
  /** TSP Protocol Version */
  int version_id;
  /** TSP Asynchronous Channel Id  */
  unsigned int channel_id;
  /** List of PGI for which we want extended informations */
  int pgi<>;
};

/**
 * Answer to @ref TSP_request_extended_information_t.
 */
struct TSP_answer_extended_information_t {
  /** TSP Protocol Version */
  int version_id;
  /** TSP Asynchronous Channel Id  */
  unsigned int channel_id;
  /** The Symbols extended informations */
  TSP_sample_symbol_extended_info_list_t extsymbols;
  /** Status */
  TSP_status_t status;		
};

/*
  FIXME : Use it when the consumer will be able
  to ask for a given endianity
  enum TSP_endianity_t 
  {
  TSP_PSEUDO_XDR_LITTLE_ENDIAN,
  TSP_XDR_BIG_ENDIAN
  };*/

/**
 * TSP Request Sample.
 * This request is used by a TSP consumer
 * to negotiate the symbol he wants.
 */
struct TSP_request_sample_t
{
  /** TSP Protocol Version */
  int version_id;
  /** TSP Asynchronous Channel Id  */
  unsigned int channel_id;
  unsigned int feature_words[4];
  int consumer_timeout;
  TSP_sample_symbol_info_list_t symbols;
  
  /* TSP_endianity_t data_endianity; FIXME : implementer */
};

/**
 * TSP Sample Answer.
 * The answer to TSP Request Sample.
 */
struct TSP_answer_sample_t
{
  /** TSP Protocol Version */
  int version_id;
  /** TSP Asynchronous Channel Id  */
  unsigned int channel_id;
  
  int provider_timeout;
  int provider_group_number;
  /** 
   * The list of symbol available from the provider 
   * this list may be incomplete if it coming
   * either from a TSP request filtered information
   * or from Request Sample
   */
  TSP_sample_symbol_info_list_t symbols;
  double base_frequency;
  int max_period; 
  int max_client_number; 
  int current_client_number; 
  TSP_status_t status;		
  
  /*unsigned int feature_words[4]; FIXME*/
  /* TSP_endianity_t data_endianity; FIXME : implementer */


};

/**
 * TSP Request sample initialisation.
 * Ask the provider to begin sampling with
 * the previously negotiated request sample.
 */
struct TSP_request_sample_init_t
{
  /** TSP Protocol Version */
  int version_id;
  /** TSP Asynchronous Channel Id  */
  unsigned int channel_id;
};

/**
 * TSP Answer sample initialisation.
 * Answer to TSP request sample initialisation,
 * the provider indicates where the consumer
 * may read the data sample stream from.
 */
struct TSP_answer_sample_init_t
{
  /** TSP Protocol Version */
  int version_id;
  /** TSP Asynchronous Channel Id  */
  unsigned int channel_id;
  /** 
   * The (string-encoded) address where
   * the consumer may find the sample data stream
   */ 
  string data_address<>;
  /**
   * Status of the answer.
   */
  TSP_status_t status;			
};


/**
 * TSP Request sample destroy.
 * Ask the provider to finalize
 * the sample process.
 */
struct TSP_request_sample_destroy_t
{
  /** TSP Protocol Version */
  int version_id;
  /** TSP Asynchronous Channel Id  */
  unsigned int channel_id;	
};

/**
 * TSP Answer sample destroy.
 * Answer to TSP Request sample destroy.
 */
struct TSP_answer_sample_destroy_t
{
  /** TSP Protocol Version */
  int version_id;
  /** TSP Asynchronous Channel Id  */
  unsigned int channel_id;
  /** TSP Status */		
  int status;
};

/**
 * TSP asynchronous sample definition.
 * This is to be used in 
 * Asynchronous sample write and read request.
 */
struct TSP_async_sample_t {
  /** 
   * The provider global index (PGI).
   * This a provider-side index which uniquely
   * identify a symbol. Each symbol provided by
   * a TSP provider should have a unique PGI assigned
   * by the provider.
   */
  int provider_global_index;
  /**
   * Opaque data container.
   */
  opaque data<>;
};

struct TSP_request_async_sample_t {
 /** TSP Protocol Version */
  int version_id;
  /** TSP Asynchronous Channel Id  */
  unsigned int channel_id;
  /** The asynchronous sample */
  TSP_async_sample_t async_sample;
};

struct TSP_answer_async_sample_t {
  /** TSP Protocol Version */
  int version_id;
  /** TSP Asynchronous Channel Id  */
  unsigned int channel_id;
  /** The asynchronous sample */
  TSP_async_sample_t async_sample;
  /** TSP Status */		
  int status;
};


/* This structure is not part of TSP Protocol*/
struct TSP_provider_info_t{
  string info<>;
};




/*------------------------------------------------------*/
/*		RPC Functions for TSP Provider		*/
/*------------------------------------------------------*/
program TSP_RPC {

	version TSP_RPC_VERSION_INITIAL {

	

	/* This function is not part of TSP Protocol */	
	TSP_provider_info_t 	TSP_PROVIDER_INFORMATION(void) = 101;

	
	/* TSP Protocol */	

	TSP_answer_open_t 		     TSP_REQUEST_OPEN(TSP_request_open_t) = 102;
	
	int 				     TSP_REQUEST_CLOSE(TSP_request_close_t) = 103;
	
	TSP_answer_sample_t		     TSP_REQUEST_INFORMATION(TSP_request_information_t) = 104;
	
	TSP_answer_feature_t		     TSP_REQUEST_FEATURE(TSP_request_feature_t) = 105;
	
	TSP_answer_sample_t		     TSP_REQUEST_SAMPLE(TSP_request_sample_t) = 106;
	
	TSP_answer_sample_init_t	     TSP_REQUEST_SAMPLE_INIT(TSP_request_sample_init_t) = 107;
	
	TSP_answer_sample_destroy_t	     TSP_REQUEST_SAMPLE_DESTROY(TSP_request_sample_destroy_t) = 108;
	
	int				     TSP_EXEC_FEATURE(TSP_exec_feature_t) = 109;
	
	int 				     TSP_REQUEST_ASYNC_SAMPLE_WRITE(TSP_async_sample_t ) = 110;

	TSP_async_sample_t                   TSP_REQUEST_ASYNC_SAMPLE_READ(TSP_async_sample_t) = 111;

	TSP_answer_sample_t		     TSP_REQUEST_FILTERED_INFORMATION(TSP_request_information_t , int , string) = 112;
	TSP_answer_extended_information_t    TSP_REQUEST_EXTENDED_INFORMATION(TSP_request_extended_information_t ) = 113;
	

	} = 1;
} = 0x31230010 ;


/** @} */

#ifdef RPC_CLNT
%
%int tsp_wrap_rpc_clnt_set_timeout(CLIENT *client, int timeout)
%{
%	static struct timeval	tv;
%    
%	/* RPC timeout definition */
%	/* ----------------------- */
%	tv.tv_sec  = timeout;
%	tv.tv_usec = 0;
%	return clnt_control(client, CLSET_TIMEOUT, (char *)&tv);
%}
#endif

%#define TSP_STRACE_RPC_ERROR(cl, pResult) if(!pResult)  {  STRACE_ERROR("%s", clnt_sperror(cl, ""));  }
