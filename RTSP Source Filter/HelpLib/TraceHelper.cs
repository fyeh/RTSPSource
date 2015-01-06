

using System;
using System.Diagnostics;
using System.IO;
using System.Reflection;
using System.Xml;
using log4net;
using log4net.Config;
using Microsoft.Win32;

namespace HelpLib
{
    /// <summary>
    /// Make life easier when using log4net.  This class adds some
    /// helper functions that make it easier to use a debug log.
    /// You must include the log4net.config in your application 
    /// directory.  Look at the default log4net.config for more
    /// information on configuration settings
    /// 
    /// DEBUG - Anything and everything, use this as you wish, inside loops to dump 
    /// data whatever.  Just beware that over logging will slow the system and may 
    /// hide some potential performance issues.
    ///  
    /// INFO - Should be used to provide useful state information only.  This level 
    /// should filter out all the debug information and provide the a view of what 
    /// the application is doing.  It should not be used in loops or methods that 
    /// are called frequently.
    ///  
    /// WARNING - Use when something unexpected has happened, but the application 
    /// continues to operated under normal conditions. This should be the level 
    /// that the product ships with, so it should provide a pretty accurate glimpse 
    /// of what is going wrong in the system.
    ///  
    /// ERROR - Typically you should not write ERROR level messages to the logfile, 
    /// that is the job of the exceptionhelper.  However, if you have a failed 
    /// operation that is not got by exceptionhelper then you would write an ERROR 
    /// level message. There is no need to do something like 
    /// Tracehelper.WriteError(exception.Message) let exception helper handle it with 
    /// ExceptionHelper.HandleException(exception)
    ///  
    /// FATAL - This is an application crash, I only use this in my unhandled 
    /// exception helper, this is the highest error level and should only be 
    /// used when the application is crashing, or failed to complete a critical 
    /// task that my leave the application in an unknown state.
    /// 
    /// </summary>
    public sealed class TraceHelper
    {
        private static bool _initialized;
        private static int _today;
        private static int _tryCount;

        /// <summary>
        /// default logger for video
        /// </summary>
        private static ILog _defaultLogger;

        private const string DefaultLogFileConfiguration =
            "" + "<log4net> " + "<appender name=\"LogFileAppender\" type=\"log4net.Appender.RollingFileAppender\"> " +
            "<File type=\"log4net.Util.PatternString\" value=\"%property{LogName}\" /> " +
            "<appendToFile value=\"true\" /> " +
            "<rollingStyle value=\"Date\" /> " +
            "<datePattern value=\"yyyy-MM-dd.lo\\g\" /> " +
            "<maxSizeRollBackups value=\"30\" /> " +
            "<maximumFileSize value=\"15MB\" /> " +
            "<staticLogFileName value=\"true\" /> " +
            "<immediateFlush value=\"true\" /> " +
            "<lockingModel type=\"log4net.Appender.FileAppender+MinimalLock\" /> " +
            "<layout type=\"log4net.Layout.PatternLayout\"> " +
            "<conversionPattern value=\"%date; %logger; %thread; %-5level; %message%newline\" /> " +
            "</layout> " +
            "</appender> " +
            "   <appender name=\"DebugAppender\" type=\"log4net.Appender.DebugAppender\"> " +
            "<immediateFlush value=\"true\" /> " +
            "<layout type=\"log4net.Layout.PatternLayout\"> " +
            "<conversionPattern value=\"%logger [%thread]  %-5level- %message%newline\" /> " +
            "</layout> " +
            "</appender> " +
            "  <appender name=\"ColoredConsoleAppender\" type=\"log4net.Appender.ColoredConsoleAppender\">"+
    "<target value=\"Console.Error\" />" +
    "<mapping>" +
    "  <level value=\"FATAL\" />" +
    " <foreColor value=\"Red\" />" +
    "  <backColor value=\"White\" />" +
    "</mapping>" +
    "<mapping>" +
    "  <level value=\"ERROR\" />" +
    "  <foreColor value=\"Red, HighIntensity\" />" +
    "</mapping>" +
    "<mapping>" +
    "  <level value=\"WARN\" />" +
    "  <foreColor value=\"Yellow\" />" +
    "</mapping>" +
    "<mapping>" +
    "  <level value=\"INFO\" />" +
    "  <foreColor value=\"Cyan\" />" +
    "</mapping>" +
    "<mapping>" +
    "  <level value=\"DEBUG\" />" +
    "  <foreColor value=\"Green\" />" +
    "</mapping>" +
    "<layout type=\"log4net.Layout.PatternLayout\">" +
    "  <conversionPattern value=\"%logger [%thread]  %-5level- %message%newline\" />" +
    "</layout>" +
  "</appender>" +
            "  <root>" + 
            "    <level value=\"INFO\" />" +
            "    <appender-ref ref=\"LogFileAppender\" />" + 
            "    <appender-ref ref=\"ColoredConsoleAppender\" />" +
             "   <appender-ref ref=\"DebugAppender\" />" +
             "  </root> " +
            "</log4net>";

        public enum LogLevel
        {
            /// <summary>
            /// This is the most verbose level, and is not typically turned on
            /// except to trace bugs
            /// </summary>
            Debug,

            /// <summary>
            /// This is the default trace level, use this to log things that are
            /// not excepted events or high priority infomation
            /// </summary>
            Warning,

            /// <summary>
            /// Exceptions and other errors
            /// </summary>
            Error,

            /// <summary>
            /// Application crashes, or critical exceptions that the user can not
            /// ignore.
            /// </summary>
            Fatal,

            /// <summary>
            /// General trace level, between warning and 
            /// debug
            /// </summary>
            Info
        };

        /// <summary>
        /// walk the call stack until you find the first caller who is
        /// not in our namespace.
        /// </summary>
        /// <returns>logger</returns>
        private static ILog GetLogger()
        {
            if (_defaultLogger == null)
            {
                _defaultLogger = LogManager.GetLogger("root");
            }
            var log = _defaultLogger;
            var stackTrace = new StackTrace();
            var frames = stackTrace.GetFrames();
            if (frames != null)
            {
                foreach (var frame in frames)
                {
                    var method = frame.GetMethod();
                    if (GoodMethodCheck(method))
                    {
                        log = LogManager.GetLogger(method.DeclaringType);
                        break;
                    }
                }
            }
            return log ?? (_defaultLogger);
        }

        private static bool GoodMethodCheck(MethodBase method)
        {
            bool rc = true;
            if (method == null
                || method.DeclaringType == null
                || method.DeclaringType.FullName == typeof (TraceHelper).FullName)
                rc = false;
            return rc;
        }

        /// <summary>
        /// Write message to the log4net logger. I attempt to find the class name 
        /// of the guy who called this function and use that as the logger name
        /// <see cref="GetLogger"/>
        /// </summary>
        /// <param name="level">level</param>
        /// <param name="message">the text</param>
        public static void Write(LogLevel level, string message)
        {
            //add the thread id to all messages
            try
            {
                Initialize();
                if (!IsLogLevelEnabled(level))
                    return;
                var log = GetLogger();
                switch (level)
                {
                    case LogLevel.Debug:
                        log.Debug(message);
                        break;
                    case LogLevel.Error:
                        log.Error(message);
                        break;
                    case LogLevel.Fatal:
                        log.Fatal(message);
                        break;
                    case LogLevel.Info:
                        log.Info(message);
                        break;
                    case LogLevel.Warning:
                        log.Warn(message);
                        break;
                    default:
                        Console.WriteLine(message);
                        break;
                }
            }
            catch (Exception)
            {
                Console.WriteLine(message);
            }
        }

        /// <summary>
        /// This is a bit of a performance enhancer
        /// the idea is that is the defualt log does not have 
        /// the request level enabled then blow out here and
        /// save all that down stream processing.
        /// </summary>
        /// <param name="level">requested logging level</param>
        /// <returns>on or off</returns>
        private static bool IsLogLevelEnabled(LogLevel level)
        {
            bool rc = false;
            var log = _defaultLogger;
            switch (level)
            {
                case LogLevel.Debug:
                    rc = log.IsDebugEnabled;
                    break;
                case LogLevel.Error:
                    rc = log.IsErrorEnabled;
                    break;
                case LogLevel.Fatal:
                    rc = log.IsFatalEnabled;
                    break;
                case LogLevel.Info:
                    rc = log.IsInfoEnabled;
                    break;
                case LogLevel.Warning:
                    rc = log.IsWarnEnabled;
                    break;
            }
            return rc;
        }

        /// <summary>
        /// If you are using a custom appender such as RichTextBox, then
        /// you must manually call initialize otherwise the log4net subsystem
        /// won't be able to find the correct config file...
        /// </summary>
        public static void Initialize()
        {
            try
            {
                if (_today != DateTime.Now.DayOfYear)
                    DeleteOldLogFilesByDate(LogFileName);
                if (!_initialized && _tryCount < 3)
                {
                    _tryCount++;
                    string logFile = LogFileName;
                    string configFile = "log4net.config";
                    _defaultLogger = LogManager.GetLogger("root");
                    var fi = new FileInfo(configFile);
                    if (!fi.Exists)
                    {
                        var doc = new XmlDocument();
                        doc.LoadXml(DefaultLogFileConfiguration);
                        GlobalContext.Properties["LogName"] = logFile;
                        XmlConfigurator.Configure(doc.DocumentElement);
                        //string logLevel =  "Info";
                        //try
                        //{
                        //    //cant use try parse
                        //    var level=(Level)Enum.Parse(typeof(Level), logLevel, true);
                        //    ((Logger)defaultLogger.Logger).Level = level;
                        //}
                        //catch (Exception ex)
                        //{
                        //    Console.WriteLine(ex.Message);
                        //}
                    }
                    else
                    {
                        GlobalContext.Properties["LogName"] = logFile;
                        XmlConfigurator.ConfigureAndWatch(fi);
                    }
                    if (_defaultLogger.IsFatalEnabled)
                        _initialized = true;
                    Console.WriteLine("Initialize" + _initialized);
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
            }
        }

        private static void DeleteOldLogFilesByDate(string logFile)
        {
            try
            {
                if (!File.Exists(logFile))
                    return;

                _today = DateTime.Now.DayOfYear;
                int maxSizeRollBackups = 30;

                if (maxSizeRollBackups != 0)
                {
                    var fi = new FileInfo(logFile);
                    if (fi.Directory == null)
                        return;
                    foreach (var oldFile in fi.Directory.GetFiles(fi.Name + "*" + fi.Extension))
                    {
                        if (fi.FullName == oldFile.FullName)
                            continue;
                        if (oldFile.LastWriteTime.Date < DateTime.Now.AddDays(-maxSizeRollBackups).Date)
                            oldFile.Delete();
                    }
                }
            }
            catch (Exception ex)
            {
                WriteError(ex.Message);
            }
        }

        public static string LogFileName
        {
            get
            {
                string path = ".";
                try
                {
                    //get the install location of the filter
                    const string regKey = @"CLSID\{B3F5D418-CDB1-441C-9D6D-2063D5538962}\InprocServer32";
                    var registryKey = Registry.ClassesRoot.OpenSubKey(regKey);
                    var fullName = (string)registryKey.GetValue("");
                    path=Path.GetDirectoryName(fullName);
                }
                catch (Exception ex)
                {
                    ExceptionHelper.HandleException(ex);
                }

                if (!Directory.Exists(path))
                    path = ".\\Logs";

                path += "\\Logs";

                if (!Directory.Exists(path))
                    Directory.CreateDirectory(path);
                return path+@"\"+Process.GetCurrentProcess().ProcessName+".log";
            }
        }

        public static void Stop()
        {
            try
            {
                foreach (var appender in _defaultLogger.Logger.Repository.GetAppenders())
                    appender.Close();
                _initialized = false;
            }
            catch (Exception ex)
            {
                WriteError(ex.Message);
            }
        }

        /// <summary>
        /// Write a debug message, not in normal log
        /// </summary>
        /// <param name="format">string format</param>
        /// <param name="args">string parameters</param>
        public static void WriteDebug(string format, params object[] args)
        {
            Write(LogLevel.Debug, string.Format(format, args));
        }

        /// <summary>
        /// Write a informational message
        /// </summary>
        /// <param name="format">string format</param>
        /// <param name="args">string parameters</param>
        public static void WriteInfo(string format, params object[] args)
        {
            Write(LogLevel.Info, string.Format(format, args));
        }

        /// <summary>
        /// Write an error message
        /// </summary>
        /// <param name="format">string format</param>
        /// <param name="args">string args</param>
        public static void WriteError(string format, params object[] args)
        {
            Write(LogLevel.Error, string.Format(format, args));
        }

        ///<summary>
        /// Write a warning message
        /// </summary>
        /// <param name="format">string format</param>
        /// <param name="args">string args</param>
        public static void WriteWarning(string format, params object[] args)
        {
            Write(LogLevel.Warning, string.Format(format, args));
        }

        internal static void SpecialLog(string message)
        {
            if (_defaultLogger != null && _initialized)
            {
                _defaultLogger.Debug(message);
            }
        }

    }
}