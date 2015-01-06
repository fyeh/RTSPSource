
namespace SampleGrabber.Controls
{
    /// <summary>
    /// class model for the the registry settings
    /// </summary>
    public class DefaultSetting
    {
        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="name"></param>
        /// <param name="value"></param>
        /// <param name="tooltip"></param>
        public DefaultSetting(string name, string value, string tooltip)
        {
            Name = name;
            Value = value;
            Tooltip = tooltip;
        }

        /// <summary>
        /// Name of the setting
        /// </summary>
        public string Name { get; set; }

        /// <summary>
        /// Current value of the setting
        /// </summary>
        public string Value { get; set; }

        /// <summary>
        /// Tooltip for the setting
        /// </summary>
        public string Tooltip { get; set; }
    }
}
