
using System;

namespace SampleGrabber
{
    /// <summary>
    /// View for Filter status
    /// <see cref="ViewModel"/>
    /// The filter status shows a graphical view of the graph contruction. 
    /// Green indicates that the graph is correctly constructed and connected
    /// </summary>
    public partial class FilterStatus
    {
        private readonly ViewModel _viewModel;

        /// <summary>
        /// Constructor
        /// </summary>
        public FilterStatus()
        {
            InitializeComponent();
            _viewModel = ViewModel.Get();
            DataContext = _viewModel;
        }

        /// <summary>
        /// Start the graph
        /// </summary>
        /// <param name="sourceUrl">RTSPsource url</param>
        public void Start(string sourceUrl)
        {
            try
            {
                _viewModel.Start(sourceUrl);
            }
            catch (Exception e)
            {
                Console.WriteLine(e);
            }
        }

        /// <summary>
        /// Stop the graph
        /// </summary>
        public void Stop()
        {
            _viewModel.MediaControl.Stop();
            _viewModel.Running = false;
        }
    }
}
