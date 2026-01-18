//
// SetupDialogForm for LeTelescopeFFFP
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
namespace ASCOM.LeTelescopeFFFP.CoverCalibrator
{
    partial class SetupDialogForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(SetupDialogForm));
            this.cmdOK = new System.Windows.Forms.Button();
            this.cmdCancel = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.picASCOM = new System.Windows.Forms.PictureBox();
            this.label2 = new System.Windows.Forms.Label();
            this.chkTrace = new System.Windows.Forms.CheckBox();
            this.comboBoxComPort = new System.Windows.Forms.ComboBox();
            this.leTelescopeLogoBox = new System.Windows.Forms.PictureBox();
            this.gitHubLogoBox = new System.Windows.Forms.PictureBox();
            this.panel1 = new System.Windows.Forms.Panel();
            this.buttonCheck = new System.Windows.Forms.Button();
            this.version_lbl = new System.Windows.Forms.Label();
            this.FirmwareInfo = new System.Windows.Forms.Label();
            this.info_rslt_lbl = new System.Windows.Forms.Label();
            this.version_rslt_lbl = new System.Windows.Forms.Label();
            this.maxbrightness_rslt_lbl = new System.Windows.Forms.Label();
            this.maxbrightness_lbl = new System.Windows.Forms.Label();
            ((System.ComponentModel.ISupportInitialize)(this.picASCOM)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.leTelescopeLogoBox)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.gitHubLogoBox)).BeginInit();
            this.panel1.SuspendLayout();
            this.SuspendLayout();
            // 
            // cmdOK
            // 
            this.cmdOK.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.cmdOK.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.cmdOK.Location = new System.Drawing.Point(401, 174);
            this.cmdOK.Margin = new System.Windows.Forms.Padding(4);
            this.cmdOK.Name = "cmdOK";
            this.cmdOK.Size = new System.Drawing.Size(79, 28);
            this.cmdOK.TabIndex = 0;
            this.cmdOK.Text = "OK";
            this.cmdOK.UseVisualStyleBackColor = true;
            this.cmdOK.Click += new System.EventHandler(this.CmdOK_Click);
            // 
            // cmdCancel
            // 
            this.cmdCancel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.cmdCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.cmdCancel.Location = new System.Drawing.Point(401, 208);
            this.cmdCancel.Margin = new System.Windows.Forms.Padding(4);
            this.cmdCancel.Name = "cmdCancel";
            this.cmdCancel.Size = new System.Drawing.Size(79, 29);
            this.cmdCancel.TabIndex = 1;
            this.cmdCancel.Text = "Cancel";
            this.cmdCancel.UseVisualStyleBackColor = true;
            this.cmdCancel.Click += new System.EventHandler(this.CmdCancel_Click);
            // 
            // label1
            // 
            this.label1.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label1.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(224)))), ((int)(((byte)(224)))), ((int)(((byte)(224)))));
            this.label1.Location = new System.Drawing.Point(10, 9);
            this.label1.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(347, 36);
            this.label1.TabIndex = 2;
            this.label1.Text = "Le Telescope Flat Panel ASCOM driver";
            // 
            // picASCOM
            // 
            this.picASCOM.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.picASCOM.Cursor = System.Windows.Forms.Cursors.Hand;
            this.picASCOM.Image = ((System.Drawing.Image)(resources.GetObject("picASCOM.Image")));
            this.picASCOM.Location = new System.Drawing.Point(3, 211);
            this.picASCOM.Margin = new System.Windows.Forms.Padding(4);
            this.picASCOM.Name = "picASCOM";
            this.picASCOM.Size = new System.Drawing.Size(24, 26);
            this.picASCOM.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
            this.picASCOM.TabIndex = 3;
            this.picASCOM.TabStop = false;
            this.picASCOM.Click += new System.EventHandler(this.BrowseToAscom);
            this.picASCOM.DoubleClick += new System.EventHandler(this.BrowseToAscom);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(32, 106);
            this.label2.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(58, 13);
            this.label2.TabIndex = 5;
            this.label2.Text = "Comm Port";
            this.label2.TextAlign = System.Drawing.ContentAlignment.TopRight;
            // 
            // chkTrace
            // 
            this.chkTrace.AutoSize = true;
            this.chkTrace.Location = new System.Drawing.Point(113, 213);
            this.chkTrace.Margin = new System.Windows.Forms.Padding(4);
            this.chkTrace.Name = "chkTrace";
            this.chkTrace.Size = new System.Drawing.Size(69, 17);
            this.chkTrace.TabIndex = 6;
            this.chkTrace.Text = "Trace on";
            this.chkTrace.UseVisualStyleBackColor = true;
            // 
            // comboBoxComPort
            // 
            this.comboBoxComPort.FormattingEnabled = true;
            this.comboBoxComPort.Location = new System.Drawing.Point(113, 100);
            this.comboBoxComPort.Margin = new System.Windows.Forms.Padding(4);
            this.comboBoxComPort.Name = "comboBoxComPort";
            this.comboBoxComPort.Size = new System.Drawing.Size(176, 21);
            this.comboBoxComPort.TabIndex = 7;
            this.comboBoxComPort.SelectedIndexChanged += new System.EventHandler(this.comboBoxComPort_SelectedIndexChanged);
            // 
            // leTelescopeLogoBox
            // 
            this.leTelescopeLogoBox.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.leTelescopeLogoBox.Cursor = System.Windows.Forms.Cursors.Hand;
            this.leTelescopeLogoBox.Image = ((System.Drawing.Image)(resources.GetObject("leTelescopeLogoBox.Image")));
            this.leTelescopeLogoBox.Location = new System.Drawing.Point(424, 9);
            this.leTelescopeLogoBox.Margin = new System.Windows.Forms.Padding(4);
            this.leTelescopeLogoBox.Name = "leTelescopeLogoBox";
            this.leTelescopeLogoBox.Size = new System.Drawing.Size(56, 52);
            this.leTelescopeLogoBox.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
            this.leTelescopeLogoBox.TabIndex = 8;
            this.leTelescopeLogoBox.TabStop = false;
            this.leTelescopeLogoBox.Click += new System.EventHandler(this.BrowseToHome);
            // 
            // gitHubLogoBox
            // 
            this.gitHubLogoBox.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.gitHubLogoBox.Cursor = System.Windows.Forms.Cursors.Hand;
            this.gitHubLogoBox.Image = ((System.Drawing.Image)(resources.GetObject("gitHubLogoBox.Image")));
            this.gitHubLogoBox.Location = new System.Drawing.Point(35, 211);
            this.gitHubLogoBox.Margin = new System.Windows.Forms.Padding(4);
            this.gitHubLogoBox.Name = "gitHubLogoBox";
            this.gitHubLogoBox.Size = new System.Drawing.Size(28, 26);
            this.gitHubLogoBox.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
            this.gitHubLogoBox.TabIndex = 9;
            this.gitHubLogoBox.TabStop = false;
            this.gitHubLogoBox.Click += new System.EventHandler(this.BrowseToRepository);
            // 
            // panel1
            // 
            this.panel1.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(24)))), ((int)(((byte)(28)))), ((int)(((byte)(42)))));
            this.panel1.Controls.Add(this.label1);
            this.panel1.Controls.Add(this.leTelescopeLogoBox);
            this.panel1.Location = new System.Drawing.Point(-3, 0);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(496, 82);
            this.panel1.TabIndex = 10;
            // 
            // buttonCheck
            // 
            this.buttonCheck.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonCheck.Location = new System.Drawing.Point(297, 98);
            this.buttonCheck.Margin = new System.Windows.Forms.Padding(4);
            this.buttonCheck.Name = "buttonCheck";
            this.buttonCheck.Size = new System.Drawing.Size(79, 28);
            this.buttonCheck.TabIndex = 11;
            this.buttonCheck.Text = "Check";
            this.buttonCheck.UseVisualStyleBackColor = true;
            this.buttonCheck.Click += new System.EventHandler(this.buttonCheck_Click);
            // 
            // version_lbl
            // 
            this.version_lbl.AutoSize = true;
            this.version_lbl.Location = new System.Drawing.Point(45, 161);
            this.version_lbl.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.version_lbl.Name = "version_lbl";
            this.version_lbl.RightToLeft = System.Windows.Forms.RightToLeft.No;
            this.version_lbl.Size = new System.Drawing.Size(45, 13);
            this.version_lbl.TabIndex = 12;
            this.version_lbl.Text = "Version:";
            this.version_lbl.TextAlign = System.Drawing.ContentAlignment.TopRight;
            // 
            // FirmwareInfo
            // 
            this.FirmwareInfo.AutoSize = true;
            this.FirmwareInfo.Location = new System.Drawing.Point(62, 129);
            this.FirmwareInfo.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.FirmwareInfo.Name = "FirmwareInfo";
            this.FirmwareInfo.Size = new System.Drawing.Size(28, 13);
            this.FirmwareInfo.TabIndex = 14;
            this.FirmwareInfo.Text = "Info:";
            this.FirmwareInfo.TextAlign = System.Drawing.ContentAlignment.TopRight;
            // 
            // info_rslt_lbl
            // 
            this.info_rslt_lbl.AutoSize = true;
            this.info_rslt_lbl.Location = new System.Drawing.Point(110, 129);
            this.info_rslt_lbl.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.info_rslt_lbl.Name = "info_rslt_lbl";
            this.info_rslt_lbl.Size = new System.Drawing.Size(10, 13);
            this.info_rslt_lbl.TabIndex = 15;
            this.info_rslt_lbl.Text = "-";
            // 
            // version_rslt_lbl
            // 
            this.version_rslt_lbl.AutoSize = true;
            this.version_rslt_lbl.Location = new System.Drawing.Point(110, 161);
            this.version_rslt_lbl.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.version_rslt_lbl.Name = "version_rslt_lbl";
            this.version_rslt_lbl.RightToLeft = System.Windows.Forms.RightToLeft.No;
            this.version_rslt_lbl.Size = new System.Drawing.Size(10, 13);
            this.version_rslt_lbl.TabIndex = 16;
            this.version_rslt_lbl.Text = "-";
            // 
            // maxbrightness_rslt_lbl
            // 
            this.maxbrightness_rslt_lbl.AutoSize = true;
            this.maxbrightness_rslt_lbl.Location = new System.Drawing.Point(110, 177);
            this.maxbrightness_rslt_lbl.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.maxbrightness_rslt_lbl.Name = "maxbrightness_rslt_lbl";
            this.maxbrightness_rslt_lbl.RightToLeft = System.Windows.Forms.RightToLeft.No;
            this.maxbrightness_rslt_lbl.Size = new System.Drawing.Size(10, 13);
            this.maxbrightness_rslt_lbl.TabIndex = 18;
            this.maxbrightness_rslt_lbl.Text = "-";
            // 
            // maxbrightness_lbl
            // 
            this.maxbrightness_lbl.AutoSize = true;
            this.maxbrightness_lbl.Location = new System.Drawing.Point(9, 177);
            this.maxbrightness_lbl.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.maxbrightness_lbl.Name = "maxbrightness_lbl";
            this.maxbrightness_lbl.RightToLeft = System.Windows.Forms.RightToLeft.No;
            this.maxbrightness_lbl.Size = new System.Drawing.Size(81, 13);
            this.maxbrightness_lbl.TabIndex = 17;
            this.maxbrightness_lbl.Text = "Max brigthness:";
            this.maxbrightness_lbl.TextAlign = System.Drawing.ContentAlignment.TopRight;
            // 
            // SetupDialogForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.Color.White;
            this.ClientSize = new System.Drawing.Size(493, 240);
            this.Controls.Add(this.maxbrightness_rslt_lbl);
            this.Controls.Add(this.maxbrightness_lbl);
            this.Controls.Add(this.version_rslt_lbl);
            this.Controls.Add(this.info_rslt_lbl);
            this.Controls.Add(this.FirmwareInfo);
            this.Controls.Add(this.version_lbl);
            this.Controls.Add(this.buttonCheck);
            this.Controls.Add(this.panel1);
            this.Controls.Add(this.gitHubLogoBox);
            this.Controls.Add(this.comboBoxComPort);
            this.Controls.Add(this.chkTrace);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.picASCOM);
            this.Controls.Add(this.cmdCancel);
            this.Controls.Add(this.cmdOK);
            this.Font = new System.Drawing.Font("Microsoft Sans Serif", 7.8F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Margin = new System.Windows.Forms.Padding(4);
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "SetupDialogForm";
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "LeTelescopeFFFP Setup";
            this.Load += new System.EventHandler(this.SetupDialogForm_Load);
            ((System.ComponentModel.ISupportInitialize)(this.picASCOM)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.leTelescopeLogoBox)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.gitHubLogoBox)).EndInit();
            this.panel1.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button cmdOK;
        private System.Windows.Forms.Button cmdCancel;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.PictureBox picASCOM;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.CheckBox chkTrace;
        private System.Windows.Forms.ComboBox comboBoxComPort;
        private System.Windows.Forms.PictureBox leTelescopeLogoBox;
        private System.Windows.Forms.PictureBox gitHubLogoBox;
        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.Button buttonCheck;
        private System.Windows.Forms.Label version_lbl;
        private System.Windows.Forms.Label FirmwareInfo;
        private System.Windows.Forms.Label info_rslt_lbl;
        private System.Windows.Forms.Label version_rslt_lbl;
        private System.Windows.Forms.Label maxbrightness_rslt_lbl;
        private System.Windows.Forms.Label maxbrightness_lbl;
    }
}