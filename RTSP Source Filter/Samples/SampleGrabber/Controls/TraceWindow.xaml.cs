
namespace SampleGrabber.Controls
{
    /// <summary>
    /// View for Trace window
    /// <see cref="ViewModel"/>
    /// This control acts like an internal trace window, only showing
    /// log items from the core product
    /// </summary>
    public partial class TraceWindow
    {
        public TraceWindow()
        {
            InitializeComponent();
            DataContext = ViewModel.Get();
        }
    }
}
