// required for std::time_t, tm, localtime_r, strftime
#include <ctime>
#include <cstdio> // for stderr

#if !defined( __GNUG__ )
  #error "Please compile this with GNU Compiler Collection that supports c++"
#endif


// for builtins_puts, builtins_exit and byte order
#if (__GNUC__ < 4) && (__GNUC_MINOR__ < 7) 
  #error "GNU CC minimum version is 4.7"
#endif

/** DM gcc probe
* Default probing that uses ASCII terminal colours for colour coded calls. 
* Uses GCC builtins for c++ to enable most of functionality (aside of timestamp) and handles exit codes so they are visible in BASH.
* This object will allways exit with exit code from 1 to 15 - pick your poison.
* This isn't 1:1 copy of std::exception (https://en.cppreference.com/w/cpp/error/exception.html) as it doesn't implement const char *what() method. It uses ERROR_CODES instead.
* Format is meant to be usable by bash (after stripping of extra ASCII colours) with `TYPE @ TIME | 'Message' 0xERRORCODE`).
* dm_gcc_probe isn't meant to be used as-is, although no one is stopping you. Its meant to be inherited (as std::exception).
* Check (bellow) Error code construction for simple to use standard.
*/
struct dm_gcc_probe
{
	
	static constexpr const char ASCII_PANIC[]="48;2;";
	static constexpr const char ASCII_REGLR[]="38;2;";
	// TYPE @ TIME | 'Message' 0xERRORCODE |\n
	static constexpr const char PRINT_FORMT[]="\x1b[%s%sm: %s @ %s :\x1b[m '%s' 0x%X \n"; 
	static constexpr unsigned int MONITOR_EXIT=0x00000001u; // return exit code 1

  //! end_code is developer specific code that can be analyized and rechecked (for non panicking calls) : see Error code construction
	unsigned int end_code;

  /**
    @param(in) type : name to use in first textual representation, PROBE, LOG, FOO, BAR whatever. Pleace keep it capitalized.
    @param(in) message : stuff to print out as explanation. Error codes for context, messages for concept.
    @param(in) ERROR_CODE : only first 8 bits will be used as EXIT, the rest are for you.
    @param(in) panic_colour : string containing rgb values that should be set as background (if panic)
    @param(in) reglr_colour : string containing rgb values that should be set as foreground (if !panic)
  */
	dm_gcc_probe(const char *type="PROBE",const char *message = "butt", 
			unsigned int ERROR_CODE=dm_gcc_probe::MONITOR_EXIT,
			bool panic = false, 
			const char *panic_colour = "120;120;120",
			const char *reglr_colour = nullptr)
	:end_code( ERROR_CODE )
	{
		if (reglr_colour == nullptr )
		{
			reglr_colour = panic_colour;
		}
		
		char buffer[16] = {0}; // 9 is good, 16 is future proof.
		std::time_t current = std::time(nullptr);
		std::tm tm_buf;
		localtime_r(&current, &tm_buf);
		std::strftime(buffer, sizeof(buffer), "%H:%M:%S", &tm_buf);

    // write to 2 (stderr) so output can be parsed and redirected
		__builtin_fprintf(stderr, dm_gcc_probe::PRINT_FORMT,
				( panic ? dm_gcc_probe::ASCII_PANIC : dm_gcc_probe::ASCII_REGLR),
				(panic ? panic_colour : reglr_colour),
				type,
				buffer,
				message,
				ERROR_CODE | dm_gcc_probe::MONITOR_EXIT);
		if (panic)
		{
			//ensuring exit code 1
			__builtin_exit( (ERROR_CODE | dm_gcc_probe::MONITOR_EXIT) & 0xf );
		}
	}
	virtual ~dm_gcc_probe() = default;
  virtual dm_gcc_probe &operator=(const dm_gcc_probe &other) noexcept { this->end_code = other.end_code; return *this;}
};

// logging 
struct dm_gcc_logging: public dm_gcc_probe
{
  // ensures exit code 1 (on panic)
	static constexpr unsigned int code=0x1u;
	
	dm_gcc_logging(	const char *message = "logging", 
			unsigned TYPE_CODE=dm_gcc_logging::code, 
			bool panic=false)
	:dm_gcc_probe( "LOGGS", message, 
			TYPE_CODE, 
			panic, "120;240;0", "240;0;0" )
	{}
};

// logging 
struct dm_gcc_warning: public dm_gcc_probe
{
  // ensures exit code 11 (EAGAIN) on panic
	static constexpr unsigned int code=0xbu;
	
	dm_gcc_warning(	const char *message = "warning", 
			unsigned TYPE_CODE=dm_gcc_warning::code, 
			bool panic=false)
	:dm_gcc_probe( "WARNS", message, 
			TYPE_CODE, 
			panic, "0;120;240", "240;120;240" )
	{}
};

struct dm_gcc_error: public dm_gcc_probe
{
  // ensures exit code 9 (8+1) (ENOEXEC) on panic
	static constexpr unsigned int code=0x8u;
	
	dm_gcc_error(	const char *message = "whoopsie!", 
			unsigned TYPE_CODE=dm_gcc_error::code, 
			bool panic=false)
	:dm_gcc_probe( "ERROR", message, 
			TYPE_CODE, 
			panic, "204;35;120", "204;240;120"  )
	{}
};

/**
	Error code construction:
		byte 0: 	system and global exit codes
			nibble 1: system exit codes
			nibble 2: global exit codes
		byte 1:		module/object exit codes
			nibble 1: creation, initialisation and destruction exit codes
			nibble 2: setup, getup and callback exit codes
		byte 2:		system/orchestrator exit codes
			nibble 1: ressource aquisition and initialisation
			nibble 2: ressource misfile and error handling 
		byte 3:		main app codes (like 'main')
			nibble 1: dependancy exit codes
			nibble 2: stuff that happened but shouldn't happen in 'main'
*/

// will not terminate on call
#define dm_gcc_CHECK(msg) 	do { dm_gcc_probe("PROBE",msg,0x10u,false); } while(0)
#define dm_gcc_NOTICE(msg) 	do { dm_gcc_warning(msg, dm_gcc_warning::code | 0x10u, false); } while(0)
#define dm_gcc_ERROR(msg) 	do { dm_gcc_error(msg, dm_gcc_error::code | 0x10u, false); } while(0)

// will terminate on call
#define dm_gcc_PROBE(msg) 	do { dm_gcc_probe("PROBE",msg,0x20u,true);
#define dm_gcc_WARN(msg) 	do { dm_gcc_warning(msg, dm_gcc_warning::code | 0x20u, true); } while(0)
#define dm_gcc_PANIC(msg)	do { dm_gcc_error(msg, dm_gcc_error::code | 0x20u, true); } while(0)
