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

using System;
using System.IO;
using System.Text;
using System.Windows.Forms;

namespace SampleGrabber
{
    public class StringRedir : TextWriter
    {
        private TextWriter _orgWriter;
        // Redirecting Console output to RichtextBox
        private readonly RichTextBox _outBox;

        public StringRedir(RichTextBox textBox)
        {
            _orgWriter = Console.Out;
            _outBox = textBox;
        }

        public override void WriteLine(string x)
        {
            _outBox.AppendText(x);
        }

        public override Encoding Encoding
        {
            get { return Encoding.UTF8; }
        }

        protected override void Dispose(bool disposing)
        {
            Console.SetOut(_orgWriter);	// Redirect Console back to original TextWriter. 
            base.Dispose(disposing);
        }
    }
}