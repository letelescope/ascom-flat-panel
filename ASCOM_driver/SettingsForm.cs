namespace ASCOM_driver
{
    public partial class SettingsForm : Form
    {
        public SettingsForm()
        {
            InitializeComponent();
        }

        private void SettingsForm_Load(object sender, EventArgs e)
        {
            portCb.Items.Clear();
            portCb.Items.AddRange(System.IO.Ports.SerialPort.GetPortNames());
        }

        private void OkBtn_Click(object sender, EventArgs e)
        {
            Application.Exit();
        }


        private void AbortBtn_Click(object sender, EventArgs e)
        {
            Application.Exit();
        }
    }
}
