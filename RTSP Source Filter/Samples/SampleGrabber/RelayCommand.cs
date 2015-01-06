
using System;
using System.Windows.Input;

namespace SampleGrabber
{
    /// <summary>
    /// WPF helper for viewmodel commands
    /// </summary>
    public class RelayCommand : ICommand
    {
        private readonly Func<object, bool> _canExecute = (parameter) => true;
        private readonly Action<object> _execute = (parameter) => { };

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="execute"></param>
        public RelayCommand(Action<object> execute)
        {
            _execute = execute;
        }

        /// <summary>
        /// Constructor with execute and can execute methods
        /// </summary>
        /// <param name="execute"></param>
        /// <param name="canExecute"></param>
        public RelayCommand(Action<object> execute, Func<object, bool> canExecute)
            : this(execute)
        {
            _canExecute = canExecute;
        }


        public void OnCanExecuteChanged()
        {
            if (CanExecuteChanged != null)
                CanExecuteChanged(this, new EventArgs());
        }
        #region ICommand Members

        public event EventHandler CanExecuteChanged = delegate { };


        public bool CanExecute(object parameter)
        {
            return _canExecute.Invoke(parameter);
        }


        public void Execute(object parameter)
        {
            _execute.Invoke(parameter);
        }


        #endregion
    }
}