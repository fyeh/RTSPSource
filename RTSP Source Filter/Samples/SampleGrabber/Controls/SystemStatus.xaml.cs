
using System.Windows.Controls;

namespace SampleGrabber.Controls
{
    /// <summary>
    /// View for Status control
    /// <see cref="ViewModel"/>
    /// The status control display information about the current system status
    /// green indicators mean everything is installed correctly.  Red indicates
    /// that a critical component is not correctly install on the machine
    /// </summary>
    public partial class SystemStatus 
    {
        /// <summary>
        /// constructor
        /// </summary>
        public SystemStatus()
        {
            InitializeComponent();
            DataContext = ViewModel.Get();
        }
    }
}
