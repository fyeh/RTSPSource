
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.IO;
using System.Runtime.InteropServices;
using System.Security.Permissions;
using System.ServiceModel;
using System.ServiceModel.Channels;
using System.Text;
using System.Windows.Forms;
using System.Xml;

namespace HelpLib
{
    /// <summary>
    /// Static class to help deal with exceptions.
    /// <seealso cref="TraceHelper"/>
    /// </summary>
    public class ExceptionHelper
    {
        public static string usermessageTag = "UserMessage";

        #region Helpers

        /// <summary>
        /// We can safely ignore this exception, just log it and continue
        /// </summary>
        /// <param name="ex">The exception</param>
        public static void Ignore(Exception ex)
        {
            LogException(ex, string.Empty, false);
        }

        /// <summary>
        /// Ignore with a user message
        /// </summary>
        /// <param name="ex">Exception</param>
        /// <param name="userMessage">extra information</param>
        public static void Ignore(Exception ex, string userMessage)
        {
            LogException(ex, userMessage, false);
        }

        /// <summary>
        /// Automatically causes an assert and writes the error to the current trace
        /// handler.  The output includes the exception message and the stack trace.
        /// </summary>
        /// <param name="ex">The exception</param>
        public static void HandleException(Exception ex)
        {
            HandleException(ex, string.Empty);
        }

        /// <summary>
        /// Automatically causes an assert and writes the error to the current trace
        /// handler.  The output includes the exception message and the stack trace.
        /// </summary>
        /// <param name="ex">The exception</param>
        /// <param name="userMessage">Additional information</param>
        public static void HandleException(Exception ex, string userMessage)
        {
            LogException(ex, userMessage, false);
            if (!string.IsNullOrEmpty(userMessage))
            {
                ex.Data.Add(usermessageTag, userMessage);
            }
            //Debug.Assert(false, ex.Message);
        }

        /// <summary>
        /// Similar to the normal HandleException, exception this one
        /// creates a dump file.
        /// </summary>
        /// <param name="ex">The exception</param>
        public static string CriticalException(Exception ex)
        {
            return CriticalException(ex, string.Empty);
        }

        /// <summary>
        /// Similar to the normal HandleException, exception this one
        /// creates a dump file.
        /// </summary>
        /// <param name="ex">The exception</param>
        /// <param name="userMessage">Additional information</param>
        public static string CriticalException(Exception ex, string userMessage)
        {
            string dumpFile = GetDumpFilePath("CriticalException");
            try
            {
            }
            catch (Exception x)
            {
                HandleException(x);
            }
            return dumpFile;
        }

        #endregion

        #region GetMessageTextHelpers

        /// <summary>
        /// Compiles exception into a single string that contains the 
        /// relevant information
        /// </summary>
        /// <param name="ex">Exception</param>
        /// <returns>string containing the important information</returns>
        public static string GetMessageText(Exception ex)
        {
            return GetMessageText(ex, string.Empty);
        }

        /// <summary>
        /// Compiles exception into a single string that contains the 
        /// relevant information
        /// </summary>
        /// <param name="ex">Exception</param>
        /// <param name="userMessage">a user supplied message</param>
        /// <returns></returns>
        public static string GetMessageText(Exception ex, string userMessage)
        {
            string body;
            try
            {
                string totalMem;
                string availMem;
                GetMemoryStatus(out totalMem, out availMem);

                body = string.Format(txtErrorMessage
                                     , Application.ProductName
                                     , Application.ProductVersion
                                     , DateTime.Now.ToString("dd/MM/yyyy HH:mm:ss")
                                     , SystemInformation.ComputerName
                                     , SystemInformation.UserName
                                     , Environment.OSVersion
                                     , CultureInfo.CurrentCulture.Name
                                     , SystemInformation.PrimaryMonitorSize
                                     , GetSystemUpTime()
                                     , (DateTime.Now - Process.GetCurrentProcess().StartTime).TotalMinutes
                                     , totalMem
                                     , availMem
                                     , userMessage
                                     , GetExceptionDetails(ex)
                    );
            }
            catch (Exception x)
            {
                body = x.Message;
            }
            return body;
        }

        public static string GetMessageHtml(Exception ex, string userMessage)
        {
            string body;
            try
            {
                string totalMem;
                string availMem;
                GetMemoryStatus(out totalMem, out availMem);

                body = string.Format(htmlErrorMessage
                                     , Application.ProductName
                                     , Application.ProductVersion
                                     , DateTime.Now.ToString("dd/MM/yyyy HH:mm:ss")
                                     , SystemInformation.ComputerName
                                     , SystemInformation.UserName
                                     , Environment.OSVersion
                                     , CultureInfo.CurrentCulture.Name
                                     , SystemInformation.PrimaryMonitorSize
                                     , GetSystemUpTime()
                                     , (DateTime.Now - Process.GetCurrentProcess().StartTime).TotalMinutes
                                     , totalMem
                                     , availMem
                                     , userMessage
                                     , GetExceptionDetailsHtml(ex)
                    );
            }
            catch (Exception x)
            {
                body = x.Message;
            }
            return body;
        }

        private static string GetExceptionDetails(Exception ex)
        {
            string txt = string.Empty;
            int tabLevel = 1;
            Exception exception = ex;

            while (exception != null)
            {
                string errorTxt =
                    string.Format(
                        "{0}Message\r\n{0}\t{1}\r\n\r\n{0}Types\r\n{0}\t{2}\r\n\r\n{0}Call Stack\r\n{0}\t{3}\r\n\r\n{0}Target\r\n{0}\t{4}\r\n"
                        , "\t".PadRight(tabLevel, '\t')
                        , GetExceptionItem(exception, ExceptionItem.Message, ExceptionTextFormat.TXT, tabLevel)
                        , GetExceptionItem(exception, ExceptionItem.Type, ExceptionTextFormat.TXT, tabLevel)
                        , GetExceptionItem(exception, ExceptionItem.StackTrace, ExceptionTextFormat.TXT, tabLevel)
                        , GetExceptionItem(exception, ExceptionItem.Target, ExceptionTextFormat.TXT, tabLevel));
                errorTxt += "\r\n".PadRight(80, '-') + "\r\n\r\n";
                txt += errorTxt;
                tabLevel++;
                exception = exception.InnerException;
            }
            return txt;
        }

        private static string GetExceptionDetailsHtml(Exception ex)
        {
            string txt = string.Empty;
            int tabLevel = 1;
            Exception exception = ex;

            while (exception != null)
            {
                string errorTxt = GetExceptionDetailTag(exception, ExceptionItem.Message, tabLevel);
                errorTxt += GetExceptionDetailTag(exception, ExceptionItem.Type, tabLevel);
                errorTxt += GetExceptionDetailTag(exception, ExceptionItem.StackTrace, tabLevel);
                errorTxt += GetExceptionDetailTag(exception, ExceptionItem.Target, tabLevel);
                errorTxt += "<br/><hr/><p/>";
                txt += errorTxt;
                tabLevel++;
                exception = exception.InnerException;
            }
            return txt;
        }

        private static string GetExceptionDetailTag(Exception ex, ExceptionItem item, int tabLevel)
        {
            string tag = string.Empty;
            tag += "<b>" + item + "</b><br/>";
            tag += GetExceptionItem(ex, item, ExceptionTextFormat.HTML, tabLevel) + "<br/>";
            return tag;
        }

        private static void GetMemoryStatus(out string totalMem, out string availMem)
        {
            var memStatus = new MEMORYSTATUSEX();
            if (GlobalMemoryStatusEx(memStatus))
            {
                totalMem = memStatus.ullTotalPhys/(1024*1024) + "Mb";
                availMem = memStatus.ullAvailPhys/(1024*1024) + "Mb";
            }
            else
            {
                totalMem = "N/A";
                availMem = "N/A";
            }
        }

        private static string GetExceptionItem(Exception ex, ExceptionItem item, ExceptionTextFormat format,
                                               int tabLevel)
        {
            string itemText = string.Empty;
            switch (item)
            {
                case ExceptionItem.Message:
                    itemText = ex.Message;
                    break;
                case ExceptionItem.StackTrace:
                    if (!string.IsNullOrEmpty(ex.StackTrace))
                    {
                        itemText = ex.StackTrace;
                        //move the source line to the top
                        const string inTag = " in ";
                        if (itemText.Contains(inTag))
                        {
                            itemText = itemText.Substring(itemText.LastIndexOf(inTag) + 4) +
                                       "\r\n" + itemText.Substring(0, itemText.LastIndexOf(inTag));
                        }
                        if (format == ExceptionTextFormat.HTML)
                            itemText = itemText.Replace(" at ", "<br>at ");
                    }
                    break;
                case ExceptionItem.Target:
                    if (ex.TargetSite != null && !string.IsNullOrEmpty(ex.TargetSite.Name))
                        itemText = ex.TargetSite.Name;
                    break;
                case ExceptionItem.Type:
                    itemText = ex.GetType().ToString();
                    break;
            }
            itemText = string.IsNullOrEmpty(itemText) ? "N/A" : itemText;
            if (format == ExceptionTextFormat.HTML)
            {
                itemText = "<ul><li>" + itemText;
                itemText.Replace("\r\n", "<br>");
                int len = itemText.Split(new[] {"<ul>"}, StringSplitOptions.None).Length - 1;
                for (int i = 0; i < len; i++)
                    itemText += "</li></ul>";
            }
            else
            {
                itemText = "\t" + itemText.Trim();
                itemText = itemText.Replace("\r\n", "\r\n\t\t ".PadRight(tabLevel, '\t'));
            }
            return itemText;
        }

        #endregion

        /// <summary>
        /// Generic unhandled exception processor
        /// This handler automatically creates a dmp file in the
        /// application directory
        /// </summary>
        /// <param name="e"></param>
        public static void UnhandledExpection(UnhandledExceptionEventArgs e)
        {
            try
            {
                TraceHelper.Write(TraceHelper.LogLevel.Fatal, "Unhandled Exception");
                if (e.ExceptionObject is Exception)
                {
                    //to get the full power of exception logging
                    LogException(e.ExceptionObject as Exception, "Unhandled Exception", true);
                }
                if (e.IsTerminating && !(e.ExceptionObject is ObjectDisposedException))
                {
                    MessageBox.Show("A serious error has occurred and this application must exit.");
                    Application.Exit();
                }
            }
            catch
            {
            }
        }

        private static string GetDumpFilePath(string header)
        {
            return Application.CommonAppDataPath +
                   string.Format(@"\{0}_{1}.dmp", header, DateTime.Now.ToString("s").Replace(":", "_"));
        }


        #region private helpers

        /// <summary>
        /// Write the exception to the trace log
        /// </summary>
        /// <param name="ex">Exception</param>
        /// <param name="userMessage">Addition information</param>
        /// <param name="critical">log level</param>
        private static string LogException(Exception ex, string userMessage, bool critical)
        {
            var sb = new StringBuilder();
            try
            {
                if (ex is FaultException)
                    LogFaultException(ex as FaultException);
                if (ex is FileNotFoundException)
                    LogFileNotFoundException(ex as FileNotFoundException);
                TraceHelper.LogLevel level = critical ? TraceHelper.LogLevel.Fatal : TraceHelper.LogLevel.Error;
                sb.AppendLine("\r\n\r\n----------------- Exception -------------------");
                sb.Append(GetMessageText(ex, userMessage));
                if (critical)
                    sb.Append(GetModules());
                sb.AppendLine("----------------------------------------------\r\n\r\n");
                TraceHelper.Write(level, sb.ToString());
            }
            catch (Exception e)
            {
                Debug.WriteLine(e.Message);
            }
            return sb.ToString();
        }

        private static void LogFileNotFoundException(FileNotFoundException fileNotFoundException)
        {
            TraceHelper.WriteError("File Name: {0}", fileNotFoundException.FileName);
        }

        private static void LogFaultException(FaultException ex)
        {
            if (ex == null)
                return;
            try
            {
                TraceHelper.WriteError("Fault Exception");
                TraceHelper.WriteError("Fault Message: {0}", ex.Message);
                TraceHelper.WriteError("Fault Action: {0}", ex.Action);
                TraceHelper.WriteError("Fault Code: {0}-{1}", ex.Code.Name, ex.Code);
                TraceHelper.WriteError("Fault Reason: {0}", ex.Reason);
                MessageFault fault = ex.CreateMessageFault();
                if (fault.HasDetail)
                {
                    XmlReader reader = fault.GetReaderAtDetailContents();
                    if (reader != null && reader.Name == "ExceptionDetail")
                    {
                        var detail = fault.GetDetail<ExceptionDetail>();
                        if (detail != null)
                        {
                            TraceHelper.WriteError("-Detail Message: {0}", detail.Message);
                            TraceHelper.WriteError("-Detail Stack: {0}", detail.StackTrace);
                        }
                    }
                }
            }
            catch (Exception e)
            {
                TraceHelper.WriteError("Error handling Fault Exception: {0}", e.Message);
            }
        }

        /// <summary>
        /// System up time
        /// </summary>
        /// <returns></returns>
        private static TimeSpan GetSystemUpTime()
        {
            var ret = new TimeSpan();
            try
            {
                //This is causing a 30second hang, comment out for now DMH 6/3/10
                //var upTime = new PerformanceCounter("System", "System Up Time");
                //upTime.NextValue();
                //ret= TimeSpan.FromSeconds(upTime.NextValue());
            }
            catch (Exception ex)
            {
                HandleException(ex);
            }
            return ret;
        }

        /// <summary>
        /// All the loaded modules
        /// </summary>
        /// <returns></returns>
        public static string GetModules()
        {
            var modules = new StringBuilder();
            modules.AppendLine("Loaded Modules:");
            Process thisProcess = Process.GetCurrentProcess();
            var list = new SortedList<string, string>();
            foreach (ProcessModule module in thisProcess.Modules)
            {
                list.Add(module.FileName, "\t" + module.FileName + " " + module.FileVersionInfo.FileVersion);
            }
            foreach (string item in list.Values)
                modules.AppendLine(item);
            return modules.ToString();
        }

        #endregion

        #region interop

        [return: MarshalAs(UnmanagedType.Bool)]
        [DllImport("kernel32.dll", CharSet = CharSet.Auto, SetLastError = true)]
        private static extern bool GlobalMemoryStatusEx([In, Out] MEMORYSTATUSEX lpBuffer);

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Auto)]
        private class MEMORYSTATUSEX
        {
            public uint dwLength;
            public uint dwMemoryLoad;
            public ulong ullTotalPhys;
            public ulong ullAvailPhys;
            public ulong ullTotalPageFile;
            public ulong ullAvailPageFile;
            public ulong ullTotalVirtual;
            public ulong ullAvailVirtual;
            public ulong ullAvailExtendedVirtual;

            public MEMORYSTATUSEX()
            {
                dwLength = (uint) Marshal.SizeOf(typeof (MEMORYSTATUSEX));
            }
        }

        #endregion

        #region Nested type: ExceptionItem

        private enum ExceptionItem
        {
            StackTrace,
            Message,
            Type,
            Target
        }

        #endregion

        #region Nested type: ExceptionTextFormat

        private enum ExceptionTextFormat
        {
            HTML,
            TXT
        }

        #endregion

        #region HTML_Error_Message

        private static string htmlErrorMessage = "" +
                                                 "<html>" +
                                                 "<head>" +
                                                 "<title>Exception Handler</title>" +
                                                 "</head>" +
                                                 "<body>" +
                                                 "<table>" +
                                                 "<tr><th align=right>Application</th><td></td><td>{0}</td></tr>" +
                                                 "<tr><th align=right>Version</th><td></td><td>{1}</td></tr>" +
                                                 "<tr><th align=right>Date</th><td></td><td>{2}</td></tr>" +
                                                 "<tr><th align=right>Computer name</th><td></td><td>{3}</td></tr>" +
                                                 "<tr><th align=right>User name</th><td></td><td>{4}</td></tr>" +
                                                 "<tr><th align=right>OS</th><td></td><td>{5}</td></tr>" +
                                                 "<tr><th align=right>Culture</th><td></td><td>{6}</td></tr>" +
                                                 "<tr><th align=right>Resolution</th><td></td><td>{7}</td></tr>" +
                                                 "<tr><th align=right>System up time</th><td></td><td>{8}</td></tr>" +
                                                 "<tr><th align=right>App up time</th><td></td><td>{9}</td></tr>" +
                                                 "<tr><th align=right>Total memory</th><td></td><td>{10}</td></tr>" +
                                                 "<tr><th align=right>Available memory</th><td></td><td>{11}</td></tr>" +
                                                 "<tr><th align=right>User Message</th><td></td><td>{12}</td></tr>" +
                                                 "</table>" +
                                                 "<p />" +
                                                 "<b>Exception Details</b>" +
                                                 "<hr />" +
                                                 "{13}" +
                                                 "</body>" +
                                                 "</html>";

        #endregion

        #region TXT_Error_Message

        private static string txtErrorMessage = "" +
                                                "Exception Handler\r\n" +
                                                "\r\n" +
                                                "Application:      {0}\r\n" +
                                                "Version:          {1}\r\n" +
                                                "Date:             {2}\r\n" +
                                                "Computer name:    {3}\r\n" +
                                                "User name:        {4}\r\n" +
                                                "OS:               {5}\r\n" +
                                                "Culture:          {6}\r\n" +
                                                "Resolution:       {7}\r\n" +
                                                "System up time:   {8}\r\n" +
                                                "App up time:      {9}\r\n" +
                                                "Total memory:     {10}\r\n" +
                                                "Available memory: {11}\r\n" +
                                                "User Message:     {12}\r\n" +
                                                "\r\n\r\n" +
                                                "---------------------------------------\r\n" +
                                                "Exception Details\r\n" +
                                                "{13}\r\n";

        #endregion

        #region Nested type: HRESULT

        public static class HRESULT
        {
            [SecurityPermission(SecurityAction.Demand, Flags = SecurityPermissionFlag.UnmanagedCode)]
            public static void Check(int hr)
            {
                Marshal.ThrowExceptionForHR(hr);
            }
        }

        #endregion
    }
}