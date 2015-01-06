
using System;
using System.Windows.Data;

namespace SampleGrabber.Controls
{
    /// <summary>
    /// This class checks the log entry to see if it 
    /// begins with the work error.  If it does set 
    /// the foreground to red.
    /// <example>
    ///  <Style x:Key="ItemContStyle" TargetType="{x:Type ListViewItem}">
    ///      <Setter Property="Foreground" Value="{Binding Converter={StaticResource errorHighlight}}" />
    ///  </Style>
    /// ...
    ///  <ListView Grid.Row="1" ItemsSource="{Binding Log}" ItemContainerStyle="{StaticResource ItemContStyle}" />
    /// </example>
    /// </summary>
    public class TraceLogItemConverter : IValueConverter
    {
        /// <summary>
        /// Converter
        /// </summary>
        /// <param name="value"></param>
        /// <param name="targetType"></param>
        /// <param name="parameter"></param>
        /// <param name="culture"></param>
        /// <returns></returns>
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            if (value != null && value.ToString().ToLower().StartsWith("error"))
                return "Red";
            return "Black";
        }

        /// <summary>
        /// Convert back - not used
        /// </summary>
        /// <param name="value"></param>
        /// <param name="targetType"></param>
        /// <param name="parameter"></param>
        /// <param name="culture"></param>
        /// <returns></returns>
        public object ConvertBack(object value, Type targetType, object parameter,
                                  System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}