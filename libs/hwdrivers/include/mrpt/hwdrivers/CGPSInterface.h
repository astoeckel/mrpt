/* +---------------------------------------------------------------------------+
   |                     Mobile Robot Programming Toolkit (MRPT)               |
   |                          http://www.mrpt.org/                             |
   |                                                                           |
   | Copyright (c) 2005-2015, Individual contributors, see AUTHORS file        |
   | See: http://www.mrpt.org/Authors - All rights reserved.                   |
   | Released under BSD License. See details in http://www.mrpt.org/License    |
   +---------------------------------------------------------------------------+ */

#ifndef CGPSInterface_H
#define CGPSInterface_H

#include <mrpt/obs/CObservationGPS.h>
#include <mrpt/poses/CPoint3D.h>
#include <mrpt/hwdrivers/CSerialPort.h>
#include <mrpt/utils/CDebugOutputCapable.h>
#include <mrpt/hwdrivers/CGenericSensor.h>
#include <mrpt/obs/obs_frwds.h>

namespace mrpt
{
	namespace hwdrivers
	{
		/** A parser of NMEA commands, for connecting to a GPS by a serial port.
		  * This class also supports more advanced GPS equipped with RTK corrections. See the JAVAD/TopCon extra initialization parameters.
		  *
		  *  \code
		  *  PARAMETERS IN THE ".INI"-LIKE CONFIGURATION STRINGS:
		  * -------------------------------------------------------
		  *   [supplied_section_name]
		  *    COM_port_WIN = COM3
		  *    COM_port_LIN = ttyS0
		  *    baudRate     = 4800   // The baudrate of the communications (typ. 4800 bauds)
		  *    pose_x       = 0      // 3D position of the sensed point relative to the robot (meters)
		  *    pose_y       = 0
		  *    pose_z       = 0
		  *    customInit   =       // See below for possible values
		  *
		  *	   // The next parameters are optional and will be used only
		  *    // if customInit=="JAVAD" to enable/configure the usage of RTK corrections:
		  *    //JAVAD_rtk_src_port=/dev/ser/b
		  *    //JAVAD_rtk_src_baud=9600
		  *    //JAVAD_rtk_format=cmr
		  *
		  *  \endcode
		  *
		  * - customInit: Custom commands to send, depending on the sensor. Valid values are:
		  *		- "": Empty string
		  *		- "JAVAD": JAVAD or TopCon devices. Extra initialization commands will be sent.
		  *		- "TopCon": A synonymous with "JAVAD".
		  *
		  *  VERSIONS HISTORY:
		  *		-9/JUN/2006: First version (JLBC)
		  *		-4/JUN/2008: Added virtual methods for device-specific initialization commands.
		  *		-10/JUN/2008: Converted into CGenericSensor class (there are no inhirited classes anymore).
		  *		-7/DEC/2012: Added public static method to parse NMEA strings.
		  *		-17/JUN/2014: Added GGA feedback.
		  *
		  *  The next picture summarizes existing MRPT classes related to GPS / GNSS devices (CGPSInterface, CNTRIPEmitter, CGPS_NTRIP):
		  *
		  *  <div align=center> <img src="mrpt_gps_classes_usage.png"> </div>
		  *
		  *  \note Verbose debug info will be dumped to cout if the environment variable "MRPT_HWDRIVERS_VERBOSE" is set to "1", or if you call CGenericSensor::enableVerbose(true)
		  *
		  * \sa CGPS_NTRIP, CNTRIPEmitter
		  * \ingroup mrpt_hwdrivers_grp
		  */
		class HWDRIVERS_IMPEXP CGPSInterface : public utils::CDebugOutputCapable, public CGenericSensor
		{
			DEFINE_GENERIC_SENSOR(CGPSInterface)

		public:
			/** Constructor
			  * \param BUFFER_LENGTH The size of the communications buffer (default value should be fine always)
			  */
			CGPSInterface( int BUFFER_LENGTH = 500, mrpt::hwdrivers::CSerialPort *outPort = NULL, mrpt::synch::CCriticalSection *csOutPort = NULL);

			/** Destructor
			  */
			virtual ~CGPSInterface();

			// See docs in parent class
			void  doProcess();

			/** Returns true if communications work.
			*/
			bool  isGPS_connected();

			/** Returns true if the last message from the GPS indicates that the signal from sats has been acquired.
			*/
			bool  isGPS_signalAcquired();

			void  setSerialPortName(const std::string &COM_port);  //!< Set the serial port to use (COM1, ttyUSB0, etc).
			std::string getSerialPortName() const;  //!< Get the serial port to use (COM1, ttyUSB0, etc).

			inline void setExternCOM( CSerialPort *outPort, mrpt::synch::CCriticalSection *csOutPort )
			{ m_out_COM = outPort; m_cs_out_COM = csOutPort; }

			inline bool isAIMConfigured() { return m_AIMConfigured; }

			/** Parses one line of NMEA data from a GPS receiver, and writes the recognized fields (if any) into an observation object.
			  * Recognized frame types are: "GGA" and "RMC".
			  * \return true if some new data field has been correctly parsed and inserted into out_obs
			  */
			static bool parse_NMEA(const std::string &cmd_line, mrpt::obs::CObservationGPS &out_obs, const bool verbose=false);

			/** Gets the latest GGA command or an empty string if no newer GGA command was received since the last call to this method.
			  * \param[in] reset If set to true, will empty the GGA cache so next calls will return an empty string if no new frame is received.
			  */
			std::string getLastGGA(bool reset=true);

		protected:
			/** Implements custom messages to be sent to the GPS unit just after connection and before normal use.
			  *  Returns false or raise an exception if something goes wrong.
			  */
			bool OnConnectionEstablished();

			CSerialPort		m_COM;

            // MAR'11 -------------------------------------
			CSerialPort		                *m_out_COM;
            mrpt::synch::CCriticalSection   *m_cs_out_COM;
			// --------------------------------------------

			poses::CPoint3D	m_sensorPose;

			std::string		m_customInit;

			/** See the class documentation at the top for expected parameters */
			void  loadConfig_sensorSpecific(
				const mrpt::utils::CConfigFileBase &configSource,
				const std::string	  &iniSection );

			/** If not empty, will send a cmd "set,/par/pos/pd/port,...". Example value: "/dev/ser/b" */
			void setJAVAD_rtk_src_port( const std::string &s) { m_JAVAD_rtk_src_port = s; }

			/** Only used when "m_JAVAD_rtk_src_port" is not empty */
			void setJAVAD_rtk_src_baud(unsigned int baud) { m_JAVAD_rtk_src_baud = baud; }

			/** Only used when "m_JAVAD_rtk_src_port" is not empty: format of RTK corrections: "cmr", "rtcm", "rtcm3", etc. */
			void setJAVAD_rtk_format(const std::string &s) {m_JAVAD_rtk_format=s;}

			/** Set Advanced Input Mode for the primary port.
                This can be used to send RTK corrections to the device using the same port that it's used for the commands.
                The RTK correction stream must be re-packaged into a special frame with prefix ">>" */
            bool setJAVAD_AIM_mode();

			/** Unset Advanced Input Mode for the primary port and use it only as a command port. */
            bool unsetJAVAD_AIM_mode();

            // MAR'11 -------------------------------------
            inline bool useExternCOM() const { return (m_out_COM!=NULL); }
            // --------------------------------------------

		private:
			std::string 	m_COMname;
			int				m_COMbauds;
			bool			m_GPS_comsWork;
			bool			m_GPS_signalAcquired;
			int				m_BUFFER_LENGTH;

			char			*m_buffer;
			size_t			m_bufferLength;
			size_t			m_bufferWritePos;

			std::string		m_JAVAD_rtk_src_port; 	//!< If not empty, will send a cmd "set,/par/pos/pd/port,...". Example value: "/dev/ser/b"
			unsigned int	m_JAVAD_rtk_src_baud; 	//!< Only used when "m_JAVAD_rtk_src_port" is not empty
			std::string		m_JAVAD_rtk_format; 	//!< Only used when "m_JAVAD_rtk_src_port" is not empty: format of RTK corrections: "cmr", "rtcm", "rtcm3", etc.

			// MAR'11 -----------------------------------------
			bool            m_useAIMMode;           //!< Use this mode for receive RTK corrections from a external source through the primary port
            // ------------------------------------------------
			mrpt::system::TTimeStamp      m_last_timestamp;

			// MAR'11 -----------------------------------------
			bool            m_AIMConfigured;        //!< Indicates if the AIM has been properly set up.
			double          m_data_period;          //!< The period in seconds which the data should be provided by the GPS
            // ------------------------------------------------

			/** Returns true if the COM port is already open, or try to open it in other case.
			  * \return true if everything goes OK, or false if there are problems opening the port.
			  */
			bool  tryToOpenTheCOM();

			/** Process data in "m_buffer" to extract GPS messages, and remove them from the buffer.
			  */
			void  processBuffer();

			/** Process a complete string from the GPS:
			  */
			void  processGPSstring( const std::string &s);

			/* A private copy of the last received gps datum:
			 */
			mrpt::obs::CObservationGPS	            m_latestGPS_data;
			mrpt::obs::CObservationGPS::TUTCTime   m_last_UTC_time;

			std::string   m_last_GGA; //!< Used in getLastGGA()
			
			void JAVAD_sendMessage(const char*str, bool waitForAnswer = true); //!< Private auxiliary method. Raises exception on error.

		}; // end class

	} // end namespace
} // end namespace

#endif
