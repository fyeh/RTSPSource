
namespace SampleGrabber.Controls
{
    /// <summary>
    /// View for default settings
    /// <see cref="ViewModel"/>
    /// </summary>
    public partial class DefaultSettings 
    {
        /// <summary>
        /// Constructor
        /// </summary>
        public DefaultSettings()
        {
            InitializeComponent();
            DataContext = ViewModel.Get();
        }
    }
}
