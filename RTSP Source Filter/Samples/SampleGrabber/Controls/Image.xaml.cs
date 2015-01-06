
namespace SampleGrabber.Controls
{
    /// <summary>
    /// View for image control
    /// <see cref="ViewModel"/>
    /// The image control allows the user to get the current frame
    /// from the stream via the sample grabber filter
    /// </summary>
    public partial class Image 
    {
        /// <summary>
        /// Constructor
        /// </summary>
        public Image()
        {
            InitializeComponent();
            DataContext = ViewModel.Get();
        }
    }
}
