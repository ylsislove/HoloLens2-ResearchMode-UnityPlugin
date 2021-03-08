using System;
using System.Runtime.InteropServices;
using TMPro;
using UnityEngine;

public class IMUManager : MonoBehaviour
{
    [StructLayout(LayoutKind.Sequential)]
    struct OUTPUT
    {
        public float x;
        public float y;
        public float z;
    }

#if ENABLE_WINMD_SUPPORT
    [DllImport("HL2RmUnityPlugin", EntryPoint = "StartStreaming", CallingConvention = CallingConvention.StdCall)]
    public static extern void StartStreaming();
    [DllImport("HL2RmUnityPlugin", EntryPoint = "GetAccelSample", CallingConvention = CallingConvention.StdCall)]
    public static extern int GetAccelSample(IntPtr pv1);
    [DllImport("HL2RmUnityPlugin", EntryPoint = "GetGyroSample", CallingConvention = CallingConvention.StdCall)]
    public static extern int GetGyroSample(IntPtr pv1);
    [DllImport("HL2RmUnityPlugin", EntryPoint = "GetMagSample", CallingConvention = CallingConvention.StdCall)]
    public static extern int GetMagSample(IntPtr pv1);
#endif

    public TextMeshPro accelTextMesh;
    public TextMeshPro gyroTextMesh;
    public TextMeshPro magTextMesh;

    private void Start()
    {
#if ENABLE_WINMD_SUPPORT
        StartStreaming();
#endif
    }

    private void Update()
    {
#if ENABLE_WINMD_SUPPORT
        int sizeOut = Marshal.SizeOf(typeof(OUTPUT));
        IntPtr pBuffOut = Marshal.AllocHGlobal(sizeOut);

        int result = GetAccelSample(pBuffOut);
        OUTPUT pOut = (OUTPUT)Marshal.PtrToStructure(pBuffOut, typeof(OUTPUT));

        if (accelTextMesh != null)
        {
            accelTextMesh.text = $"Accel: {pOut.x} {pOut.y} {pOut.z}";
        }

        result = GetGyroSample(pBuffOut);
        pOut = (OUTPUT)Marshal.PtrToStructure(pBuffOut, typeof(OUTPUT));

        if (gyroTextMesh != null)
        {
            gyroTextMesh.text = $"Gyro: {pOut.x} {pOut.y} {pOut.z}";
        }

        result = GetMagSample(pBuffOut);
        pOut = (OUTPUT)Marshal.PtrToStructure(pBuffOut, typeof(OUTPUT));

        if (magTextMesh != null)
        {
            magTextMesh.text = $"Mag: {pOut.x} {pOut.y} {pOut.z}";
        }
#endif
    }
}
