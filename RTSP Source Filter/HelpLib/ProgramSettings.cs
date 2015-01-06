
using System;
using System.Collections.Specialized;
using System.Configuration;
using System.IO;
using System.Xml;

namespace HelpLib
{
    /// <summary>
    /// Helper class to read and write app.config files
    /// </summary>
    public class ProgramSettings
    {
        private const string ConfigNode = "Video.Settings";
        private static ProgramSettings _instance;
        private readonly NameValueCollection _nvc;
        private readonly static object LockObj = new object(); ///Object reference for thread saftey.

        private ProgramSettings()
        {
            _nvc = new NameValueCollection();
        }


        private static ProgramSettings GetInstance()
        {
            try
            {
                lock (typeof (ProgramSettings))
                {
                    if (_instance == null)
                    {
                        _instance = new ProgramSettings();
                    }
                }
            }
            catch (Exception ex)
            {
                ExceptionHelper.HandleException(ex);
            }
            return _instance;
        }

        /// <summary>
        /// Get setting from app config
        /// </summary>
        /// <param name="name">setting name</param>
        /// <returns>setting value</returns>
        public static string GetSetting(string name)
        {
            return GetSetting(name, string.Empty);
        }

        /// <summary>
        /// Get setting from custom app config file
        /// </summary>
        /// <param name="configPath">path to app.config file</param>
        /// <param name="name">setting name</param>
        /// <param name="defaultValue">default value</param>
        /// <returns>setting value</returns>
        public static string GetSetting(string configPath, string name, string defaultValue)
        {
            //Use this code to seperate settings by dll
            //ns = new System.Diagnostics.StackTrace().GetFrame(1).GetMethod().ReflectedType.Namespace;
            //if (ns.Contains(configNode))
            const string ns = ConfigNode;
            return GetSetting(configPath, ns, name, defaultValue);
        }

        /// <summary>
        /// Get Setting w default value
        /// </summary>
        /// <param name="name">setting name</param>
        /// <param name="defaultValue">default value</param>
        /// <returns>value</returns>
        public static string GetSetting(string name, string defaultValue)
        {
            //Use this code to seperate settings by dll
            //ns = new System.Diagnostics.StackTrace().GetFrame(1).GetMethod().ReflectedType.Namespace;
            //if (ns.Contains(configNode))
            const string ns = ConfigNode;
            return GetSetting(string.Empty, ns, name, defaultValue);
        }

        /// <summary>
        /// Get setting from custom app.config
        /// </summary>
        /// <param name="cPath">path to config file</param>
        /// <param name="location">section name</param>
        /// <param name="name">setting name</param>
        /// <param name="defaultValue">default value</param>
        /// <returns>setting value</returns>
        public static string GetSetting(string cPath, string location, string name, string defaultValue)
        {
            string ret = string.Empty;
            try
            {
                //first check the cache
                string lookup = location + ":" + name;
                ProgramSettings ps = GetInstance();
                if (ps._nvc.Get(lookup) != null)
                {
                    lock(LockObj)
                        ret = ps._nvc.Get(lookup);
                }
                else
                {
                    lock (LockObj)
                    {
                        string configFile = string.Empty;
                        try
                        {
                            var doc = new XmlDocument();
                            configFile = LoadConfigFile(cPath, doc);
                            string nodeName =
                                string.Format(@"descendant::applicationSettings/{0}/setting[@name='{1}']/value",
                                              location, name);
                            ret = GetNodeValue(ps, lookup, doc, nodeName, ret);
                        }
                        catch (Exception ex)
                        {
                            ExceptionHelper.HandleException(ex, string.Format("Can't find config file {0}", configFile));
                            throw;
                        }

                    }
                }
            }
            catch (Exception ex)
            {
                ret = string.Empty;
                ExceptionHelper.HandleException(ex);
                if (string.IsNullOrEmpty(defaultValue))
                    throw;
            }

            if (string.IsNullOrEmpty(ret) && !string.IsNullOrEmpty(defaultValue))
                ret = defaultValue;
            return ret;
        }

        private static string LoadConfigFile(string cPath, XmlDocument doc)
        {
            string configFile;
            if (cPath.Length > 0)
            {
                configFile = cPath;
            }
            else if (!string.IsNullOrEmpty(ConfigurationManager.AppSettings["ConfigPath"]))
            {
                configFile = ConfigurationManager.AppSettings["ConfigPath"];
            }
            else
            {
                Configuration c = ConfigurationManager.OpenExeConfiguration(ConfigurationUserLevel.None);
                configFile = c.FilePath;
            }
            if(File.Exists(configFile))
                doc.Load(configFile);
            return configFile;
        }

        private static string GetNodeValue(ProgramSettings ps, string lookup, XmlDocument doc, string nodeName, string ret)
        {
            XmlNode node = doc.SelectSingleNode(nodeName);
            if (node != null)
            {
                ret = node.InnerText;
                ps._nvc.Set(lookup, ret);
            }
            return ret;
        }

        /// <summary>
        /// Write setting to app config
        /// </summary>
        /// <param name="configPath">config file path</param>
        /// <param name="name">setting name</param>
        /// <param name="newValue">new value</param>
        public static void SetSetting(string configPath, string name, string newValue)
        {
            //Use this code to seperate settings by dll
            //ns = new System.Diagnostics.StackTrace().GetFrame(1).GetMethod().ReflectedType.Namespace;
            //if (ns.Contains(configNode))
            const string ns = ConfigNode;
            SetSetting(configPath, ns, name, newValue);
        }

        /// <summary>
        /// Write setting to app config
        /// </summary>
        /// <param name="name">setting name</param>
        /// <param name="newValue">setting value</param>
        public static void SetSetting(string name, string newValue)
        {
            //Use this code to seperate settings by dll
            //ns = new System.Diagnostics.StackTrace().GetFrame(1).GetMethod().ReflectedType.Namespace;
            //if (ns.Contains(configNode))
            const string ns = ConfigNode;
            SetSetting(string.Empty, ns, name, newValue);
        }

        /// <summary>
        /// write setting to app config
        /// </summary>
        /// <param name="cPath">path to config file</param>
        /// <param name="location">app section</param>
        /// <param name="name">setting</param>
        /// <param name="newValue">new value</param>
        public static void SetSetting(string cPath, string location, string name, string newValue)
        {
            //first check the cache
            string lookup = location + ":" + name;
            ProgramSettings ps = GetInstance();
            string nodeName = string.Format(@"descendant::applicationSettings/{0}/setting[@name='{1}']/value",
                                            location, name);

            var doc = new XmlDocument();
            string configPath;
            if (cPath.Length > 0)
            {
                configPath = cPath;
            }
            else
            {
                Configuration c =
                    ConfigurationManager.OpenExeConfiguration(ConfigurationUserLevel.None);
                configPath = c.FilePath;
            }
            doc.Load(configPath);

            XmlNode node = doc.SelectSingleNode(nodeName);
            if (node == null)
            {
                string parent = string.Format(@"descendant::applicationSettings/{0}", location);
                XmlNode parentNode = doc.SelectSingleNode(parent);
                if (parentNode != null)
                {
                    XmlNode newNode = doc.CreateNode(XmlNodeType.Element, "setting", string.Empty);
                    XmlAttribute nameAttribute = doc.CreateAttribute("name");
                    nameAttribute.Value = name;
                    if (newNode.Attributes != null) newNode.Attributes.Append(nameAttribute);
                    node = doc.CreateNode(XmlNodeType.Element, "value", string.Empty);
                    newNode.AppendChild(node);
                    parentNode.AppendChild(newNode);
                }
            }
            if (node != null)
            {
                node.InnerText = newValue;
                doc.Save(configPath);
                if (ps._nvc.Get(lookup) != null)
                    ps._nvc.Remove(lookup);
                ps._nvc.Set(lookup, newValue);
            }
        }
    }
}