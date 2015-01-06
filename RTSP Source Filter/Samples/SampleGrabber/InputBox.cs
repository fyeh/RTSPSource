
using System;
using System.IO;
using System.Linq;
using System.Windows.Forms;

namespace SampleGrabber
{
    /// <summary>
    /// A dialog box to select the RTSPsource url.  The combo
    /// box uses and auto complete source and saves all entries
    /// in the camera.dat file in the application directory.
    /// </summary>
    public partial class InputBox : Form
    {
        //The list of cameras
        private AutoCompleteStringCollection urlList;

        /// <summary>
        /// Constructor
        /// </summary>
        public InputBox()
        {
            urlList=new AutoCompleteStringCollection();
            InitializeComponent();
        }

        private void okButton_Click(object sender, EventArgs e)
        {

        }

        private void cancelButton_Click(object sender, EventArgs e)
        {

        }

        /// <summary>
        /// Static helper/access method 
        /// </summary>
        /// <returns>RTSPsource url</returns>
        public static string GetUrl()
        {
            var dlg = new InputBox();
            dlg.ShowDialog();

            if (dlg.DialogResult == DialogResult.OK)
            {
                if (!dlg.urlList.Contains(dlg.url.Text))
                {
                    dlg.urlList.Add(dlg.url.Text);
                    var urlList = (from object line in dlg.urlList select line.ToString()).ToList();
                    File.WriteAllLines("camera.dat",urlList.ToArray());
                }

                return dlg.url.Text;
            }
            return string.Empty;
        }

        /// <summary>
        /// Load our autocomplete data store
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void InputBox_Load(object sender, EventArgs e)
        {
            if(File.Exists("camera.dat"))
            {
                foreach (string camera in File.ReadAllLines("camera.dat"))
                {
                    url.Items.Add(camera);
                    urlList.Add(camera);
                }
            }
            url.AutoCompleteCustomSource = urlList;
        }
    }
}
