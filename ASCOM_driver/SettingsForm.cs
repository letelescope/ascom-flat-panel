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

        }

        private void okBtn_Click(object sender, EventArgs e)
        {
            Application.Exit();
        }


        private void abortBtn_Click(object sender, EventArgs e)
        {
            Application.Exit();
        }
    }
}
