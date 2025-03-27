namespace ASCOM_driver
{
    partial class SettingsForm
    {
        /// <summary>
        ///  Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        ///  Clean up any resources being used.
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
        ///  Required method for Designer support - do not modify
        ///  the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            okBtn = new Button();
            portTitleLbl = new Label();
            abortBtn = new Button();
            portCb = new ComboBox();
            controlsPanel = new Panel();
            driverTitleLbl = new Label();
            headerPanel = new Panel();
            mainTitleLbl = new Label();
            telescopeIconBox = new PictureBox();
            controlsPanel.SuspendLayout();
            headerPanel.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)telescopeIconBox).BeginInit();
            SuspendLayout();
            // 
            // okBtn
            // 
            okBtn.Location = new Point(129, 134);
            okBtn.Name = "okBtn";
            okBtn.Size = new Size(100, 40);
            okBtn.TabIndex = 2;
            okBtn.Text = "OK";
            okBtn.UseVisualStyleBackColor = true;
            okBtn.Click += OkBtn_Click;
            // 
            // portTitleLbl
            // 
            portTitleLbl.AutoSize = true;
            portTitleLbl.Font = new Font("Noto Sans", 10.2F, FontStyle.Bold, GraphicsUnit.Point, 0);
            portTitleLbl.Location = new Point(64, 72);
            portTitleLbl.Name = "portTitleLbl";
            portTitleLbl.Size = new Size(54, 26);
            portTitleLbl.TabIndex = 1;
            portTitleLbl.Text = "Port:";
            // 
            // abortBtn
            // 
            abortBtn.Location = new Point(252, 134);
            abortBtn.Name = "abortBtn";
            abortBtn.Size = new Size(100, 40);
            abortBtn.TabIndex = 3;
            abortBtn.Text = "Annuler";
            abortBtn.UseVisualStyleBackColor = true;
            abortBtn.Click += AbortBtn_Click;
            // 
            // portCb
            // 
            portCb.DropDownStyle = ComboBoxStyle.DropDownList;
            portCb.FormattingEnabled = true;
            portCb.Location = new Point(129, 69);
            portCb.Name = "portCb";
            portCb.Size = new Size(223, 34);
            portCb.TabIndex = 0;
            // 
            // controlsPanel
            // 
            controlsPanel.Controls.Add(driverTitleLbl);
            controlsPanel.Controls.Add(portCb);
            controlsPanel.Controls.Add(abortBtn);
            controlsPanel.Controls.Add(okBtn);
            controlsPanel.Controls.Add(portTitleLbl);
            controlsPanel.Location = new Point(0, 106);
            controlsPanel.Name = "controlsPanel";
            controlsPanel.Size = new Size(435, 177);
            controlsPanel.TabIndex = 5;
            // 
            // driverTitleLbl
            // 
            driverTitleLbl.AutoSize = true;
            driverTitleLbl.Font = new Font("Noto Sans", 10.2F, FontStyle.Bold, GraphicsUnit.Point, 0);
            driverTitleLbl.Location = new Point(36, 14);
            driverTitleLbl.Name = "driverTitleLbl";
            driverTitleLbl.Size = new Size(82, 26);
            driverTitleLbl.TabIndex = 7;
            driverTitleLbl.Text = "Version:";
            // 
            // headerPanel
            // 
            headerPanel.BackColor = Color.FromArgb(24, 28, 42);
            headerPanel.Controls.Add(mainTitleLbl);
            headerPanel.Controls.Add(telescopeIconBox);
            headerPanel.Location = new Point(0, 0);
            headerPanel.Name = "headerPanel";
            headerPanel.Size = new Size(435, 102);
            headerPanel.TabIndex = 6;
            // 
            // mainTitleLbl
            // 
            mainTitleLbl.AutoSize = true;
            mainTitleLbl.Font = new Font("Noto Sans", 13.7999992F, FontStyle.Bold, GraphicsUnit.Point, 0);
            mainTitleLbl.ForeColor = Color.FromArgb(224, 224, 224);
            mainTitleLbl.Location = new Point(120, 36);
            mainTitleLbl.Name = "mainTitleLbl";
            mainTitleLbl.Size = new Size(295, 35);
            mainTitleLbl.TabIndex = 1;
            mainTitleLbl.Text = "Flat panel ASCOM driver";
            // 
            // telescopeIconBox
            // 
            telescopeIconBox.Image = Properties.Resources.LogoLetelescope_222px_png_150x150;
            telescopeIconBox.InitialImage = Properties.Resources.LogoLetelescope_222px_png_150x150;
            telescopeIconBox.Location = new Point(0, 0);
            telescopeIconBox.Name = "telescopeIconBox";
            telescopeIconBox.Size = new Size(100, 100);
            telescopeIconBox.SizeMode = PictureBoxSizeMode.StretchImage;
            telescopeIconBox.TabIndex = 0;
            telescopeIconBox.TabStop = false;
            // 
            // SettingsForm
            // 
            AutoScaleDimensions = new SizeF(10F, 26F);
            AutoScaleMode = AutoScaleMode.Font;
            BackColor = SystemColors.Window;
            ClientSize = new Size(434, 282);
            Controls.Add(headerPanel);
            Controls.Add(controlsPanel);
            Font = new Font("Noto Sans", 10.2F, FontStyle.Regular, GraphicsUnit.Point, 0);
            FormBorderStyle = FormBorderStyle.FixedSingle;
            MaximizeBox = false;
            Name = "SettingsForm";
            SizeGripStyle = SizeGripStyle.Hide;
            Text = "Le Téléscope - Flat panel ASCOM driver";
            Load += SettingsForm_Load;
            controlsPanel.ResumeLayout(false);
            controlsPanel.PerformLayout();
            headerPanel.ResumeLayout(false);
            headerPanel.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)telescopeIconBox).EndInit();
            ResumeLayout(false);
        }

        #endregion

        private Button okBtn;
        private Label portTitleLbl;
        private Button abortBtn;
        private ComboBox portCb;
        private Panel controlsPanel;
        private Panel headerPanel;
        private PictureBox telescopeIconBox;
        private Label mainTitleLbl;
        private Label driverTitleLbl;
    }
}
