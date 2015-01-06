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

using System.Windows;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Interactivity;
namespace SampleGrabber
{
    public class ColorAnimationBehavior : TriggerAction<UIElement>
    {
        public Color FillColor
        {
            get { return (Color) GetValue(FillColorProperty); }
            set { SetValue(FillColorProperty, value); }
        }

        public static readonly DependencyProperty FillColorProperty =
            DependencyProperty.Register("FillColor", typeof (Color), typeof (ColorAnimationBehavior), null);

        public Duration Duration
        {
            get { return (Duration) GetValue(DurationProperty); }
            set { SetValue(DurationProperty, value); }
        }

        // Using a DependencyProperty as the backing store for Duration.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty DurationProperty =
            DependencyProperty.Register("Duration", typeof (Duration), typeof (ColorAnimationBehavior), null);

        protected override void Invoke(object parameter)
        {
            var storyboard = new Storyboard();
            storyboard.Children.Add(CreateColorAnimation(this.AssociatedObject, this.Duration, this.FillColor));
            storyboard.Begin();
        }

        private static ColorAnimationUsingKeyFrames CreateColorAnimation(UIElement element, Duration duration,
                                                                         Color color)
        {
            var animation = new ColorAnimationUsingKeyFrames();
            animation.KeyFrames.Add(new SplineColorKeyFrame() {KeyTime = duration.TimeSpan, Value = color});
            Storyboard.SetTargetProperty(animation, new PropertyPath("(Shape.Fill).(SolidColorBrush.Color)"));
            Storyboard.SetTarget(animation, element);
            return animation;
        }
    }
}