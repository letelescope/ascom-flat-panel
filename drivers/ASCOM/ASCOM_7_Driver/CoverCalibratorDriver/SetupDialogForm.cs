//
// SetupDialogForm for LeTelescopeFFFPV1
//
// Copyright(C) 2025 - Present, Le Télescope - Ivry sur Seine - All Rights Reserved
// Licensed under the MIT License. See the accompanying LICENSE file for terms.
//
// Description:	 Event handlers for the Setup Form of this driver. The base layout of
// this class has been generated using the Visual Studio Code ASCOM 6 driver template.
// 
// Authors:
//   - Florian Thibaud
//   - Florian Gautier		
//
using ASCOM.LocalServer;
using ASCOM.Utilities;
using System;
using System.ComponentModel;
using System.Runtime.InteropServices;
using System.Runtime.Remoting.Messaging;
using System.Windows.Forms;

namespace ASCOM.LeTelescopeFFFP.CoverCalibrator
{
    [ComVisible(false)] // Form not registered for COM!
    public partial class SetupDialogForm : Form
    {
        const string NO_PORTS_MESSAGE = "No COM ports found";
        const string DEFAULT_LABEL_VALUE = "-";
        static Guid Guid = Guid.NewGuid();
        TraceLogger tl; // Holder for a reference to the driver's trace logger

        public SetupDialogForm(TraceLogger tlDriver)
        {
            InitializeComponent();

            // Save the provided trace logger for use within the setup dialogue
            tl = tlDriver;

            // Initialise current values of user settings from the ASCOM Profile
            InitUI();

        }

        private void CmdOK_Click(object sender, EventArgs e) // OK button event handler
        {
            // Place any validation constraint checks here and update the state variables with results from the dialogue

            tl.Enabled = chkTrace.Checked;

            int wannabeeBrightness = -1;

            // Update the COM port variable if one has been selected
            if (comboBoxComPort.SelectedItem is null) // No COM port selected
            {
                tl.LogMessage("Setup OK", $"New configuration values - COM Port: Not selected");
            }
            else if (comboBoxComPort.SelectedItem.ToString() == NO_PORTS_MESSAGE)
            {
                tl.LogMessage("Setup OK", $"New configuration values - NO COM ports detected on this PC.");
            }
            else if (info_rslt_lbl.Text.Equals(DEFAULT_LABEL_VALUE))
            {
                tl.LogMessage("Setup OK", $"New configuration values - Driver did not send infos.");
            }
            else if (version_rslt_lbl.Text.Equals(DEFAULT_LABEL_VALUE))
            {
                tl.LogMessage("Setup OK", $"New configuration values - Driver did not send version.");
            }
            else if (!Int32.TryParse(maxbrightness_rslt_lbl.Text, out wannabeeBrightness))
            {
                tl.LogMessage("Setup OK", $"New configuration values - Driver did not send max brightness.");
            }
            else if (wannabeeBrightness <= 0)
            {
                tl.LogMessage("Setup OK", $"New configuration values - Maw brightness was negative.");
            }
            else // Everything looks good
            {
                CoverCalibratorHardware.comPort = (string)comboBoxComPort.SelectedItem;
                CoverCalibratorHardware.firmwareInfo = info_rslt_lbl.Text;
                CoverCalibratorHardware.firmwareVersion = version_rslt_lbl.Text;
                CoverCalibratorHardware.panelMaxBrightness = wannabeeBrightness;
                tl.LogMessage("Setup OK", $"New configuration values - ;COM Port: {comboBoxComPort.SelectedItem}; Driver info: {info_rslt_lbl.Text}; Driver version: {version_rslt_lbl.Text}; Max brightness: {wannabeeBrightness}");
            }


        }

        private void CmdCancel_Click(object sender, EventArgs e) // Cancel button event handler
        {
            Close();
        }

        private void BrowseToAscom(object sender, EventArgs e) // Click on ASCOM logo event handler
        {
            BrowseToPage("https://ascom-standards.org/");
        }

        private void BrowseToHome(object sender, EventArgs e)
        {
            BrowseToPage("https://letelescope.fr");
        }

        private void BrowseToRepository(object sender, EventArgs e)
        {
            BrowseToPage("https://github.com/letelescope/ascom-flat-panel");
        }

        private void BrowseToPage(String url)
        {
            try
            {
                System.Diagnostics.Process.Start(url);
            }
            catch (Win32Exception noBrowser)
            {
                if (noBrowser.ErrorCode == -2147467259)
                    MessageBox.Show(noBrowser.Message);
            }
            catch (Exception other)
            {
                MessageBox.Show(other.Message);
            }
        }
        private void InitUI()
        {

            // Set the trace checkbox
            chkTrace.Checked = tl.Enabled;

            // set the list of COM ports to those that are currently available
            comboBoxComPort.Items.Clear(); // Clear any existing entries
            using (Serial serial = new Serial()) // User the Se5rial component to get an extended list of COM ports
            {
                comboBoxComPort.Items.AddRange(serial.AvailableCOMPorts);
            }

            // If no ports are found include a message to this effect
            if (comboBoxComPort.Items.Count == 0)
            {
                comboBoxComPort.Items.Add(NO_PORTS_MESSAGE);
                comboBoxComPort.SelectedItem = NO_PORTS_MESSAGE;
            }

            // select the current port if possible
            if (comboBoxComPort.Items.Contains(CoverCalibratorHardware.comPort))
            {
                comboBoxComPort.SelectedItem = CoverCalibratorHardware.comPort;
            }

            if (CoverCalibratorHardware.panelMaxBrightness > 0)
            {
                maxbrightness_rslt_lbl.Text = "" + CoverCalibratorHardware.panelMaxBrightness;
            }

            if (CoverCalibratorHardware.firmwareInfo != null && !String.Empty.Equals(CoverCalibratorHardware.firmwareInfo))
            {
                info_rslt_lbl.Text = CoverCalibratorHardware.firmwareInfo;
            }

            if (CoverCalibratorHardware.firmwareVersion != null && !String.Empty.Equals(CoverCalibratorHardware.firmwareVersion))
            {
                version_rslt_lbl.Text = CoverCalibratorHardware.firmwareVersion;
            }

            if (comboBoxComPort.SelectedItem is null || comboBoxComPort.SelectedItem.ToString() == NO_PORTS_MESSAGE) // No COM port selected
            {
                buttonCheck.Enabled = false;
            }

            checkEnableOk();

            tl.LogMessage("InitUI", $"Set UI controls to Trace: {chkTrace.Checked}, COM Port: {comboBoxComPort.SelectedItem}");
        }

        private void SetupDialogForm_Load(object sender, EventArgs e)
        {
            // Bring the setup dialogue to the front of the screen
            if (WindowState == FormWindowState.Minimized)
                WindowState = FormWindowState.Normal;
            else
            {
                TopMost = true;
                Focus();
                BringToFront();
                TopMost = false;
            }
        }

        private void buttonCheck_Click(object sender, EventArgs e)
        {

            if (comboBoxComPort.SelectedItem is null || comboBoxComPort.SelectedItem.ToString() == NO_PORTS_MESSAGE) // No COM port selected
            {
                return;
            }

            string old_port = CoverCalibratorHardware.comPort;

            buttonCheck.Enabled = false;
            CoverCalibratorHardware.comPort = (string)comboBoxComPort.SelectedItem;
            CoverCalibratorHardware.SetConnected(Guid, true);

            string device_firmware_info;
            string device_firmware_version;
            int device_firmware_max_brightness;
            try
            {
                device_firmware_info = CoverCalibratorHardware.FirmwareInfoFromDevice();
                device_firmware_version = CoverCalibratorHardware.FirmwareVersionFromDevice();
                device_firmware_max_brightness = CoverCalibratorHardware.FirmwareMaxBrightnessFromDevice();

            }
            catch (Exception ex)
            {
                tl.LogMessage("Check device", $"Sending info requests to device failed {ex}");
                CoverCalibratorHardware.comPort = old_port;
                CoverCalibratorHardware.SetConnected(Guid, false);
                buttonCheck.Enabled = true;
                return;
            }


            info_rslt_lbl.Text = device_firmware_info.Replace(". Buil",".\nBuil");
            version_rslt_lbl.Text = device_firmware_version;
            maxbrightness_rslt_lbl.Text = "" + device_firmware_max_brightness;


            if (!checkEnableOk())
            {
                CoverCalibratorHardware.comPort = old_port;
                CoverCalibratorHardware.SetConnected(Guid, false);
                buttonCheck.Enabled = true;
                return;
            }

            CoverCalibratorHardware.SetConnected(Guid, false);
            buttonCheck.Enabled = true;

        }
        private bool checkEnableOk()
        {
            int current_brighthess;

            if (comboBoxComPort.SelectedItem is null)
            {
                cmdOK.Enabled = false;
            }
            else if (comboBoxComPort.SelectedItem.ToString() == NO_PORTS_MESSAGE)
            {
                cmdOK.Enabled = false;
            }
            else if (info_rslt_lbl.Text.ToString().Equals("-"))
            {
                cmdOK.Enabled = false;
            }
            else if (version_rslt_lbl.Text.ToString().Equals("-"))
            {
                cmdOK.Enabled = false;
            }
            else if (!Int32.TryParse(maxbrightness_rslt_lbl.Text, out current_brighthess))
            {
                cmdOK.Enabled = false;
            }
            else
            {
                cmdOK.Enabled = true;
            }

            return cmdOK.Enabled;
        }

        private void comboBoxComPort_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (comboBoxComPort.SelectedItem is null || comboBoxComPort.SelectedItem.ToString() == NO_PORTS_MESSAGE) // No COM port selected
            {
                buttonCheck.Enabled = false;
            }
            else
            {
                buttonCheck.Enabled = true;
            }
        }
    }
}