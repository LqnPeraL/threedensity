using System;
using System.Diagnostics;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Drawing.Text;
using System.IO;
using System.IO.Compression;
using System.Net;
using System.Text.RegularExpressions;
using System.Threading;
using System.Windows.Forms;

internal static class Program
{
    [STAThread]
    static void Main()
    {
        Application.EnableVisualStyles();
        Application.SetCompatibleTextRenderingDefault(false);
        Application.Run(new LauncherForm());
    }
}

public sealed class LauncherForm : Form
{
    const string ApiUrl = "https://api.github.com/repos/LqnPeraL/threedensity/releases/latest";
    const string FallbackZip = "https://github.com/LqnPeraL/threedensity/releases/latest/download/ThreeDensity-Win64.zip";
    const string FallbackSetup = "https://github.com/LqnPeraL/threedensity/releases/latest/download/ThreeDensitySetup.exe";
    const string LauncherFileName = "ThreeDensityLauncher.exe";

    static readonly Color Void = Color.FromArgb(7, 7, 8);
    static readonly Color Ink = Color.FromArgb(244, 241, 236);
    static readonly Color Steel = Color.FromArgb(216, 210, 200);
    static readonly Color Muted = Color.FromArgb(154, 149, 140);
    static readonly Color Ember = Color.FromArgb(255, 122, 47);
    static readonly Color EmberDeep = Color.FromArgb(196, 74, 18);

    readonly string installRoot;
    readonly string gameDir;
    readonly string versionFile;
    readonly string launcherPath;

    Image backgroundArt;
    Image lockupArt;
    Image markArt;

    string statusText = "Starting…";
    string versionText = "";
    int progressPercent;
    float shimmer;
    bool busy;
    bool showOffline;
    bool showRetry;
    bool canOffline;
    string launchPath;

    Rectangle closeRect;
    Rectangle minRect;
    Rectangle playRect;
    Rectangle retryRect;
    Rectangle progressTrack;
    int hoverChrome; // 0 none, 1 min, 2 close, 3 play, 4 retry
    Point dragOrigin;
    bool dragging;

    readonly System.Windows.Forms.Timer animTimer;

    public LauncherForm()
    {
        Text = "Three Density";
        FormBorderStyle = FormBorderStyle.None;
        StartPosition = FormStartPosition.CenterScreen;
        ClientSize = new Size(920, 540);
        BackColor = Void;
        DoubleBuffered = true;
        ShowInTaskbar = true;
        KeyPreview = true;

        try { Icon = Icon.ExtractAssociatedIcon(Application.ExecutablePath); } catch { }

        installRoot = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), "ThreeDensity");
        gameDir = Path.Combine(installRoot, "Game");
        versionFile = Path.Combine(installRoot, "version.txt");
        launcherPath = Path.Combine(installRoot, LauncherFileName);

        try { backgroundArt = LogoData.LoadBackground(); } catch { }
        try { lockupArt = LogoData.LoadLockup(); } catch { }
        try { markArt = LogoData.LoadMark(); } catch { }

        LayoutChrome();

        animTimer = new System.Windows.Forms.Timer { Interval = 16 };
        animTimer.Tick += (s, e) =>
        {
            shimmer += 0.018f;
            if (shimmer > 1.6f) shimmer = -0.2f;
            if (busy || progressPercent > 0 && progressPercent < 100) Invalidate(progressTrack);
        };
        animTimer.Start();

        Shown += (s, e) => BeginBootstrap();
        KeyDown += (s, e) =>
        {
            if (e.KeyCode == Keys.Escape) Close();
        };
    }

    protected override CreateParams CreateParams
    {
        get
        {
            CreateParams cp = base.CreateParams;
            cp.ClassStyle |= 0x00020000; // CS_DROPSHADOW
            return cp;
        }
    }

    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
            if (animTimer != null) animTimer.Dispose();
            if (backgroundArt != null) backgroundArt.Dispose();
            if (lockupArt != null) lockupArt.Dispose();
            if (markArt != null) markArt.Dispose();
        }
        base.Dispose(disposing);
    }

    void LayoutChrome()
    {
        closeRect = new Rectangle(ClientSize.Width - 52, 14, 34, 28);
        minRect = new Rectangle(ClientSize.Width - 92, 14, 34, 28);
        progressTrack = new Rectangle(48, ClientSize.Height - 118, ClientSize.Width - 96, 10);
        playRect = new Rectangle(48, ClientSize.Height - 78, 188, 44);
        retryRect = new Rectangle(248, ClientSize.Height - 78, 140, 44);
    }

    protected override void OnResize(EventArgs e)
    {
        base.OnResize(e);
        LayoutChrome();
        Invalidate();
    }

    protected override void OnPaintBackground(PaintEventArgs e)
    {
        // Fully owner-drawn — skip default fill flash.
    }

    protected override void OnPaint(PaintEventArgs e)
    {
        Graphics g = e.Graphics;
        g.SmoothingMode = SmoothingMode.AntiAlias;
        g.InterpolationMode = InterpolationMode.HighQualityBicubic;
        g.PixelOffsetMode = PixelOffsetMode.HighQuality;
        g.TextRenderingHint = TextRenderingHint.ClearTypeGridFit;

        Rectangle bounds = ClientRectangle;
        using (var brush = new SolidBrush(Void))
            g.FillRectangle(brush, bounds);

        if (backgroundArt != null)
        {
            float scale = Math.Max((float)bounds.Width / backgroundArt.Width, (float)bounds.Height / backgroundArt.Height);
            int w = (int)(backgroundArt.Width * scale);
            int h = (int)(backgroundArt.Height * scale);
            var dest = new Rectangle((bounds.Width - w) / 2, (bounds.Height - h) / 2, w, h);
            g.DrawImage(backgroundArt, dest);
        }

        using (var veil = new LinearGradientBrush(bounds, Color.FromArgb(210, 7, 7, 8), Color.FromArgb(120, 7, 7, 8), LinearGradientMode.ForwardDiagonal))
            g.FillRectangle(veil, bounds);
        using (var bottom = new LinearGradientBrush(
            new Point(0, bounds.Height - 220), new Point(0, bounds.Height),
            Color.FromArgb(0, 7, 7, 8), Color.FromArgb(245, 7, 7, 8)))
            g.FillRectangle(bottom, 0, bounds.Height - 220, bounds.Width, 220);
        using (var top = new LinearGradientBrush(
            new Point(0, 0), new Point(0, 90),
            Color.FromArgb(180, 7, 7, 8), Color.FromArgb(0, 7, 7, 8)))
            g.FillRectangle(top, 0, 0, bounds.Width, 90);

        // Ember accent line under chrome
        using (var emberPen = new Pen(Color.FromArgb(90, Ember), 1f))
            g.DrawLine(emberPen, 48, 56, bounds.Width - 48, 56);

        DrawChromeButtons(g);

        int copyTop = 188;
        if (lockupArt != null)
        {
            int lw = Math.Min(480, lockupArt.Width);
            int lh = Math.Max(1, (int)(lockupArt.Height * (lw / (float)lockupArt.Width)));
            var logoRect = new Rectangle(48, 72, lw, lh);
            g.DrawImage(lockupArt, logoRect);
            copyTop = logoRect.Bottom + 18;
        }
        else if (markArt != null)
        {
            g.DrawImage(markArt, new Rectangle(48, 80, 72, 72));
            using (var font = new Font("Segoe UI", 28f, FontStyle.Bold))
            using (var brush = new SolidBrush(Ink))
                g.DrawString("THREE DENSITY", font, brush, 136, 92);
            copyTop = 168;
        }

        using (var font = new Font("Segoe UI", 11f, FontStyle.Regular))
        using (var brush = new SolidBrush(Steel))
            g.DrawString("Third-person combat", font, brush, 52, copyTop);

        if (!string.IsNullOrEmpty(versionText))
        {
            using (var font = new Font("Segoe UI", 8.5f, FontStyle.Bold))
            using (var brush = new SolidBrush(Ember))
                g.DrawString(versionText.ToUpperInvariant(), font, brush, 52, copyTop + 26);
        }

        // Status panel
        var statusBox = new Rectangle(48, bounds.Height - 168, bounds.Width - 96, 36);
        using (var font = new Font("Segoe UI", 10.5f))
        using (var brush = new SolidBrush(Ink))
        {
            var sf = new StringFormat { Trimming = StringTrimming.EllipsisCharacter, FormatFlags = StringFormatFlags.NoWrap };
            g.DrawString(statusText ?? "", font, brush, statusBox, sf);
        }

        DrawProgress(g);
        if (showOffline) DrawActionButton(g, playRect, "PLAY OFFLINE", hoverChrome == 3, true);
        if (showRetry) DrawActionButton(g, retryRect, "RETRY", hoverChrome == 4, false);

        // Outer frame
        using (var frame = new Pen(Color.FromArgb(55, 255, 255, 255), 1f))
            g.DrawRectangle(frame, 0, 0, bounds.Width - 1, bounds.Height - 1);
        using (var inner = new Pen(Color.FromArgb(40, Ember), 1f))
            g.DrawRectangle(inner, 1, 1, bounds.Width - 3, bounds.Height - 3);
    }

    void DrawChromeButtons(Graphics g)
    {
        DrawChromeHit(g, minRect, hoverChrome == 1, false);
        DrawChromeHit(g, closeRect, hoverChrome == 2, true);

        using (var pen = new Pen(hoverChrome == 1 ? Ink : Steel, 1.6f))
        {
            int cx = minRect.X + minRect.Width / 2;
            int cy = minRect.Y + minRect.Height / 2;
            g.DrawLine(pen, cx - 6, cy + 4, cx + 6, cy + 4);
        }

        using (var pen = new Pen(hoverChrome == 2 ? Color.White : Steel, 1.6f))
        {
            int cx = closeRect.X + closeRect.Width / 2;
            int cy = closeRect.Y + closeRect.Height / 2;
            g.DrawLine(pen, cx - 6, cy - 6, cx + 6, cy + 6);
            g.DrawLine(pen, cx + 6, cy - 6, cx - 6, cy + 6);
        }
    }

    void DrawChromeHit(Graphics g, Rectangle r, bool hot, bool danger)
    {
        if (!hot) return;
        Color fill = danger ? Color.FromArgb(200, 160, 40, 30) : Color.FromArgb(90, 255, 255, 255);
        using (var path = Rounded(r, 4))
        using (var brush = new SolidBrush(fill))
            g.FillPath(brush, path);
    }

    void DrawProgress(Graphics g)
    {
        using (var track = new SolidBrush(Color.FromArgb(55, 255, 255, 255)))
        using (var path = Rounded(progressTrack, 4))
            g.FillPath(track, path);

        int fillW = (int)(progressTrack.Width * (Math.Max(0, Math.Min(100, progressPercent)) / 100.0));
        if (fillW > 0)
        {
            var fillRect = new Rectangle(progressTrack.X, progressTrack.Y, Math.Max(8, fillW), progressTrack.Height);
            using (var path = Rounded(fillRect, 4))
            using (var brush = new LinearGradientBrush(fillRect, Ember, EmberDeep, LinearGradientMode.Horizontal))
            {
                g.FillPath(brush, path);
                // shimmer
                float sx = fillRect.X + fillRect.Width * shimmer;
                using (var shine = new LinearGradientBrush(
                    new Point((int)sx - 40, fillRect.Y),
                    new Point((int)sx + 40, fillRect.Y),
                    Color.FromArgb(0, 255, 255, 255),
                    Color.FromArgb(90, 255, 255, 255)))
                {
                    shine.SetSigmaBellShape(0.5f);
                    g.SetClip(path);
                    g.FillRectangle(shine, fillRect);
                    g.ResetClip();
                }
            }
        }

        using (var font = new Font("Segoe UI", 8.5f, FontStyle.Bold))
        using (var brush = new SolidBrush(Muted))
            g.DrawString(progressPercent + "%", font, brush, progressTrack.Right - 36, progressTrack.Y - 18);
    }

    void DrawActionButton(Graphics g, Rectangle r, string label, bool hot, bool primary)
    {
        Color c1 = primary ? Ember : Color.FromArgb(48, 48, 50);
        Color c2 = primary ? EmberDeep : Color.FromArgb(28, 28, 30);
        if (hot)
        {
            c1 = ControlPaint.Light(c1);
            c2 = ControlPaint.Light(c2);
        }

        using (var path = Angled(r, 12))
        using (var brush = new LinearGradientBrush(r, c1, c2, LinearGradientMode.ForwardDiagonal))
        {
            g.FillPath(brush, path);
            if (primary)
            {
                using (var glow = new Pen(Color.FromArgb(hot ? 160 : 80, Ember), 1.2f))
                    g.DrawPath(glow, path);
            }
            else
            {
                using (var border = new Pen(Color.FromArgb(70, 255, 255, 255), 1f))
                    g.DrawPath(border, path);
            }
        }

        using (var font = new Font("Segoe UI", 10f, FontStyle.Bold))
        using (var brush = new SolidBrush(Color.White))
        {
            var sf = new StringFormat { Alignment = StringAlignment.Center, LineAlignment = StringAlignment.Center };
            g.DrawString(label, font, brush, r, sf);
        }
    }

    static GraphicsPath Rounded(Rectangle r, int radius)
    {
        var path = new GraphicsPath();
        int d = radius * 2;
        path.AddArc(r.X, r.Y, d, d, 180, 90);
        path.AddArc(r.Right - d, r.Y, d, d, 270, 90);
        path.AddArc(r.Right - d, r.Bottom - d, d, d, 0, 90);
        path.AddArc(r.X, r.Bottom - d, d, d, 90, 90);
        path.CloseFigure();
        return path;
    }

    static GraphicsPath Angled(Rectangle r, int cut)
    {
        var path = new GraphicsPath();
        path.AddPolygon(new[]
        {
            new Point(r.X, r.Y),
            new Point(r.Right - cut, r.Y),
            new Point(r.Right, r.Y + cut),
            new Point(r.Right, r.Bottom),
            new Point(r.X + cut, r.Bottom),
            new Point(r.X, r.Bottom - cut)
        });
        return path;
    }

    protected override void OnMouseDown(MouseEventArgs e)
    {
        base.OnMouseDown(e);
        if (e.Button != MouseButtons.Left) return;

        if (closeRect.Contains(e.Location)) { Close(); return; }
        if (minRect.Contains(e.Location)) { WindowState = FormWindowState.Minimized; return; }
        if (showOffline && playRect.Contains(e.Location)) { LaunchGameAndExit(); return; }
        if (showRetry && retryRect.Contains(e.Location)) { BeginBootstrap(); return; }

        dragging = true;
        dragOrigin = e.Location;
    }

    protected override void OnMouseMove(MouseEventArgs e)
    {
        base.OnMouseMove(e);
        if (dragging)
        {
            Point screen = PointToScreen(e.Location);
            Location = new Point(screen.X - dragOrigin.X, screen.Y - dragOrigin.Y);
            return;
        }

        int next = 0;
        if (minRect.Contains(e.Location)) next = 1;
        else if (closeRect.Contains(e.Location)) next = 2;
        else if (showOffline && playRect.Contains(e.Location)) next = 3;
        else if (showRetry && retryRect.Contains(e.Location)) next = 4;

        if (next != hoverChrome)
        {
            hoverChrome = next;
            Cursor = next > 0 ? Cursors.Hand : Cursors.Default;
            Invalidate();
        }
    }

    protected override void OnMouseUp(MouseEventArgs e)
    {
        base.OnMouseUp(e);
        dragging = false;
    }

    protected override void OnMouseLeave(EventArgs e)
    {
        base.OnMouseLeave(e);
        if (hoverChrome != 0)
        {
            hoverChrome = 0;
            Cursor = Cursors.Default;
            Invalidate();
        }
    }

    void BeginBootstrap()
    {
        if (busy) return;
        busy = true;
        showOffline = false;
        showRetry = false;
        progressPercent = 0;
        Invalidate();

        ThreadPool.QueueUserWorkItem(state =>
        {
            try
            {
                ServicePointManager.SecurityProtocol = SecurityProtocolType.Tls12;
                Directory.CreateDirectory(installRoot);

                SetStatus("Preparing launcher…", 4);
                EnsureInstalledLauncherAndShortcuts();

                SetStatus("Checking GitHub for updates…", 10);
                ReleaseInfo release = FetchLatestRelease();
                string installed = ReadInstalledVersion();
                launchPath = FindExe(gameDir);
                UiSetVersion(string.IsNullOrEmpty(installed) ? release.Tag : installed);

                bool needsInstall = string.IsNullOrEmpty(launchPath) || !File.Exists(launchPath);
                bool needsUpdate = needsInstall || !VersionsEqual(installed, release.Tag);

                if (needsUpdate)
                {
                    if (needsInstall)
                        SetStatus("Downloading Three Density " + release.Tag + "…", 15);
                    else
                        SetStatus("Update found: " + release.Tag + " · downloading…", 15);

                    string zipUrl = string.IsNullOrEmpty(release.ZipUrl) ? FallbackZip : release.ZipUrl;
                    string zipPath = Path.Combine(installRoot, "ThreeDensity-Win64.zip");
                    Download(zipUrl, zipPath);

                    SetStatus("Installing " + release.Tag + "…", 82);
                    InstallGameFromZip(zipPath);
                    try { File.Delete(zipPath); } catch { }

                    launchPath = FindExe(gameDir);
                    if (string.IsNullOrEmpty(launchPath) || !File.Exists(launchPath))
                        throw new InvalidOperationException("Install finished but threedensity.exe was not found.");

                    File.WriteAllText(versionFile, release.Tag ?? "");
                    UiSetVersion(release.Tag);
                    TryRefreshLauncher(release.SetupUrl);
                    EnsureInstalledLauncherAndShortcuts();
                    SetStatus("Updated to " + release.Tag + ". Launching…", 96);
                }
                else
                {
                    SetStatus("Up to date (" + installed + "). Launching…", 90);
                }

                Thread.Sleep(450);
                Invoke(new Action(LaunchGameAndExit));
            }
            catch (Exception ex)
            {
                launchPath = FindExe(gameDir);
                canOffline = !string.IsNullOrEmpty(launchPath) && File.Exists(launchPath);
                SetStatus("Update check failed: " + ex.Message + (canOffline ? "  ·  You can still play offline." : ""), 0);
                Invoke(new Action(() =>
                {
                    busy = false;
                    showRetry = true;
                    showOffline = canOffline;
                    Invalidate();
                }));
            }
        });
    }

    void EnsureInstalledLauncherAndShortcuts()
    {
        string current = Application.ExecutablePath;
        try
        {
            if (!PathsEqual(current, launcherPath))
                File.Copy(current, launcherPath, true);
        }
        catch { }

        string target = File.Exists(launcherPath) ? launcherPath : current;
        CreateShortcut(Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.DesktopDirectory), "Three Density.lnk"), target);
        string startMenu = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.StartMenu), "Programs");
        Directory.CreateDirectory(startMenu);
        CreateShortcut(Path.Combine(startMenu, "Three Density.lnk"), target);
    }

    void TryRefreshLauncher(string setupUrl)
    {
        if (string.IsNullOrEmpty(setupUrl)) return;
        try
        {
            string temp = Path.Combine(installRoot, "ThreeDensitySetup.download.exe");
            DownloadQuiet(setupUrl, temp);
            string pending = Path.Combine(installRoot, "ThreeDensityLauncher.pending.exe");
            if (File.Exists(pending)) try { File.Delete(pending); } catch { }
            File.Move(temp, pending);

            string bat = Path.Combine(installRoot, "update-launcher.bat");
            string script =
                "@echo off\r\n" +
                "timeout /t 2 /nobreak >nul\r\n" +
                "copy /y \"" + pending + "\" \"" + launcherPath + "\" >nul\r\n" +
                "del \"" + pending + "\" >nul 2>&1\r\n" +
                "del \"%~f0\" >nul 2>&1\r\n";
            File.WriteAllText(bat, script);
            Process.Start(new ProcessStartInfo(bat)
            {
                WorkingDirectory = installRoot,
                WindowStyle = ProcessWindowStyle.Hidden,
                CreateNoWindow = true
            });
        }
        catch { }
    }

    void InstallGameFromZip(string zipPath)
    {
        string staging = Path.Combine(installRoot, "Game.staging");
        if (Directory.Exists(staging)) Directory.Delete(staging, true);
        Directory.CreateDirectory(staging);
        ZipFile.ExtractToDirectory(zipPath, staging);

        if (Directory.Exists(gameDir))
        {
            try { Directory.Delete(gameDir, true); }
            catch
            {
                string old = gameDir + ".old-" + DateTime.Now.Ticks;
                Directory.Move(gameDir, old);
                try { Directory.Delete(old, true); } catch { }
            }
        }
        Directory.Move(staging, gameDir);
    }

    void LaunchGameAndExit()
    {
        if (string.IsNullOrEmpty(launchPath) || !File.Exists(launchPath))
            launchPath = FindExe(gameDir);
        if (string.IsNullOrEmpty(launchPath) || !File.Exists(launchPath))
        {
            SetStatus("Game executable not found.", 0);
            busy = false;
            showRetry = true;
            Invalidate();
            return;
        }

        Process.Start(new ProcessStartInfo(launchPath)
        {
            WorkingDirectory = Path.GetDirectoryName(launchPath)
        });
        Close();
    }

    string ReadInstalledVersion()
    {
        try
        {
            if (File.Exists(versionFile)) return File.ReadAllText(versionFile).Trim();
        }
        catch { }
        return "";
    }

    static bool VersionsEqual(string a, string b)
    {
        return string.Equals(NormalizeTag(a), NormalizeTag(b), StringComparison.OrdinalIgnoreCase);
    }

    static string NormalizeTag(string tag)
    {
        if (string.IsNullOrEmpty(tag)) return "";
        tag = tag.Trim();
        if (tag.StartsWith("v", StringComparison.OrdinalIgnoreCase)) tag = tag.Substring(1);
        return tag;
    }

    static bool PathsEqual(string a, string b)
    {
        try { return string.Equals(Path.GetFullPath(a), Path.GetFullPath(b), StringComparison.OrdinalIgnoreCase); }
        catch { return string.Equals(a, b, StringComparison.OrdinalIgnoreCase); }
    }

    sealed class ReleaseInfo
    {
        public string Tag;
        public string ZipUrl;
        public string SetupUrl;
    }

    ReleaseInfo FetchLatestRelease()
    {
        using (var client = new WebClient())
        {
            client.Headers[HttpRequestHeader.UserAgent] = "ThreeDensityLauncher";
            client.Headers[HttpRequestHeader.Accept] = "application/vnd.github+json";
            string json = client.DownloadString(ApiUrl);

            var info = new ReleaseInfo();
            Match tag = Regex.Match(json, "\"tag_name\"\\s*:\\s*\"([^\"]+)\"");
            if (tag.Success) info.Tag = tag.Groups[1].Value;

            info.ZipUrl = FindAssetUrl(json, "ThreeDensity-Win64.zip") ?? FallbackZip;
            info.SetupUrl = FindAssetUrl(json, "ThreeDensitySetup.exe") ?? FallbackSetup;
            if (string.IsNullOrEmpty(info.Tag)) info.Tag = "latest";
            return info;
        }
    }

    static string FindAssetUrl(string json, string fileName)
    {
        int idx = json.IndexOf(fileName, StringComparison.OrdinalIgnoreCase);
        if (idx < 0) return null;
        int urlKey = json.LastIndexOf("browser_download_url", idx, StringComparison.OrdinalIgnoreCase);
        if (urlKey < 0) return null;
        int http = json.IndexOf("https://", urlKey, StringComparison.OrdinalIgnoreCase);
        if (http < 0) return null;
        int end = json.IndexOf("\"", http);
        if (end < 0) return null;
        return json.Substring(http, end - http);
    }

    static string FindExe(string root)
    {
        if (!Directory.Exists(root)) return null;
        string launcher = Path.Combine(root, "Windows", "threedensity.exe");
        if (File.Exists(launcher)) return launcher;
        string[] matches = Directory.GetFiles(root, "threedensity.exe", SearchOption.AllDirectories);
        if (matches.Length > 0) return matches[0];
        matches = Directory.GetFiles(root, "threedensity-Win64-Shipping.exe", SearchOption.AllDirectories);
        return matches.Length > 0 ? matches[0] : null;
    }

    void Download(string url, string dest)
    {
        using (var client = new WebClient())
        {
            client.Headers[HttpRequestHeader.UserAgent] = "ThreeDensityLauncher";
            client.DownloadProgressChanged += (s, e) =>
            {
                int pct = 15 + (int)(e.ProgressPercentage * 0.65);
                SetStatus(string.Format("Downloading… {0}%", e.ProgressPercentage), pct);
            };
            var done = new ManualResetEvent(false);
            Exception error = null;
            client.DownloadFileCompleted += (s, e) =>
            {
                error = e.Error;
                done.Set();
            };
            client.DownloadFileAsync(new Uri(url), dest);
            done.WaitOne();
            if (error != null) throw error;
        }
    }

    void DownloadQuiet(string url, string dest)
    {
        using (var client = new WebClient())
        {
            client.Headers[HttpRequestHeader.UserAgent] = "ThreeDensityLauncher";
            client.DownloadFile(url, dest);
        }
    }

    static void CreateShortcut(string lnkPath, string target)
    {
        Type t = Type.GetTypeFromProgID("WScript.Shell");
        object shell = Activator.CreateInstance(t);
        object shortcut = t.InvokeMember("CreateShortcut", System.Reflection.BindingFlags.InvokeMethod, null, shell, new object[] { lnkPath });
        Type st = shortcut.GetType();
        st.InvokeMember("TargetPath", System.Reflection.BindingFlags.SetProperty, null, shortcut, new object[] { target });
        st.InvokeMember("WorkingDirectory", System.Reflection.BindingFlags.SetProperty, null, shortcut, new object[] { Path.GetDirectoryName(target) });
        st.InvokeMember("Description", System.Reflection.BindingFlags.SetProperty, null, shortcut, new object[] { "Three Density Launcher" });
        st.InvokeMember("IconLocation", System.Reflection.BindingFlags.SetProperty, null, shortcut, new object[] { target + ",0" });
        st.InvokeMember("Save", System.Reflection.BindingFlags.InvokeMethod, null, shortcut, null);
    }

    void UiSetVersion(string tag)
    {
        if (IsDisposed) return;
        try
        {
            Invoke(new Action(() =>
            {
                versionText = string.IsNullOrEmpty(tag) ? "" : ("Build " + tag);
                Invalidate();
            }));
        }
        catch { }
    }

    void SetStatus(string text, int percent)
    {
        if (IsDisposed) return;
        try
        {
            Invoke(new Action(() =>
            {
                statusText = text;
                progressPercent = Math.Max(0, Math.Min(100, percent));
                Invalidate();
            }));
        }
        catch { }
    }
}
