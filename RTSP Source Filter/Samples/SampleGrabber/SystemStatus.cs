// IBM Confidential
//
// OCO Source Material
//
// 5725H94
//
// (C) Copyright IBM Corp. 2005,2006
//
// The source code for this program is not published or otherwise divested
// of its trade secrets, irrespective of what has been deposited with the
// U. S. Copyright Office.
//
// US Government Users Restricted Rights - Use, duplication or
// disclosure restricted by GSA ADP Schedule Contract with
// IBM Corp.

using System;
using System.ComponentModel;
using System.IO;
using Microsoft.Win32;

namespace SampleGrabber
{
    /// <summary>
    /// View model for the system status reporting
    /// toolbar.  
    /// </summary>
    public class SystemStatusCheck
    {
        /// <summary>
        /// Indicates if the filter is properly registered on the machine
        /// </summary>
        public bool FilterInstalled { get; private set; }

        /// <summary>
        /// Indicates if the plugins directory is installed correctly
        /// </summary>
        public bool PluginsFound { get; private set; }

        /// <summary>
        /// Indicates the EnduraAPI and Pelco.MPF are installed correctly
        /// </summary>
        public bool MediaBridge { get; private set; }

        /// <summary>
        /// Indicates that the required codecs are installed
        /// </summary>
        public bool Codec { get; private set; }

        /// <summary>
        /// Indicates that DirectX is installed
        /// </summary>
        public bool DirectX { get; private set; }

        /// <summary>
        /// Reports the logging level of the source filter
        /// </summary>
        public string FilterLogging { get; private set; }
        public string LogFile { get; set; }
        public bool LogOn { get { return File.Exists(LogFile); } }

        /// <summary>
        /// Check and set all the status indicators
        /// </summary>
        public SystemStatusCheck()
        {
            //check to make sure EnduraAPI is installed in the gac
            CheckAPI();

            //Check the registry for the filter
            CheckFilter();

            //check to see if we can find the plugins directory
            //first check for a 32 bit system
            CheckPlugins();

            //get the logging level of the filter
            GetLogValue();

            //Check to make sure the iyuv codec is installed
            CheckCodec();

            CheckDirectX();
        }

        /// <summary>
        /// Get the setting from the registry
        /// </summary>
        private void GetLogValue()
        {
            try
            {
                using (var key = Registry.LocalMachine.OpenSubKey("Software\\Pelco\\PelcoCommon"))
                {
                    if (key != null)
                    {
                        LogFile = (string)key.GetValue("LoggingDir", "C:\\Program Files\\Pelco\\API\\Logging\\");
                        LogFile += @"PelcoSDK.log";
                        var bits = Convert.ToInt16(key.GetValue("LogMaskBits"));
                        switch (bits)
                        {
                            case 0:
                                FilterLogging = "OFF";
                                break;
                            case 1:
                                FilterLogging = "MEMORY";
                                break;
                            case 2:
                                FilterLogging = "VERBOSE";
                                break;
                            case 4:
                                FilterLogging = "INFO";
                                break;
                            case 8:
                                FilterLogging = "ERROR";
                                break;
                            default:
                                FilterLogging = "?";
                                break;
                        }

                    }
                }
            }
            catch (Exception)
            {

            }
        }

        /// <summary>
        /// Get the location of the filter from the registry then make sure
        /// a plugins subdirectory exists
        /// </summary>
        private void CheckPlugins()
        {
            try
            {
                string path = string.Empty;
                using (
                    var key =
                        Registry.ClassesRoot.OpenSubKey(@"CLSID\{867E01EC-022B-458a-A2C2-B896EB0F442E}\InprocServer32"))
                {
                    if (key != null)
                    {
                        path = key.GetValue("").ToString();
                        path = path.Substring(0, path.LastIndexOf(@"\"));
                        PluginsFound = Directory.Exists(path + @"\Plugins");
                    }
                }
            }
            catch (Exception ex)
            {

            }
        }

        /// <summary>
        /// Get the location of the filter from the registry then make sure
        /// a enduraapi and pelco.mpf exists
        /// </summary>
        private void CheckAPI()
        {
            try
            {
                string path = string.Empty;
                using (var key =Registry.ClassesRoot.OpenSubKey(@"CLSID\{867E01EC-022B-458a-A2C2-B896EB0F442E}\InprocServer32"))
                {
                    if (key != null)
                    {
                        path = key.GetValue("").ToString();
                        path = path.Substring(0, path.LastIndexOf(@"\"));
                        MediaBridge = File.Exists(path+@"\EnduraAPI.DLL");
                    }
                }
            }
            catch (Exception ex)
            {

            }
        }

        /// <summary>
        /// Check the windows system directory to make sure direct x is installed
        /// </summary>
        private void CheckDirectX()
        {
            try
            {
                DirectX = File.Exists(@"C:\Windows\System32\D3DX9_39.dll");
            }
            catch (Exception ex)
            {

            }
        }

        /// <summary>
        /// Check the registry 
        /// </summary>
        private void CheckFilter()
        {
            try
            {
                using (var key = Registry.ClassesRoot.OpenSubKey("EnduraSource"))
                {
                    if (key != null)
                    {
                        FilterInstalled = key.GetValue("Source Filter").ToString() ==
                                          "{867E01EC-022B-458a-A2C2-B896EB0F442E}";
                    }
                }
            }
            catch (Exception ex)
            {

            }
        }

        /// <summary>
        /// Check the registry
        /// </summary>
        private void CheckCodec()
        {
            try
            {
                bool rc;
                using (var key = Registry.ClassesRoot.OpenSubKey(@"CLSID\{D76E2820-1563-11CF-AC98-00AA004C0FA9}"))
                {
                    rc = key != null;
                    if (rc)
                    {
                        Codec = File.Exists(Environment.SystemDirectory + @"\i420vfw.dll");
                    }
                }
            }
            catch (Exception ex)
            {

            }
        }
    }
}
