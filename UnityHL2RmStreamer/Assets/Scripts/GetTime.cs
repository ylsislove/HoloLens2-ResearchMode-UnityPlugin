using System;
using TMPro;
using UnityEngine;

public class GetTime : MonoBehaviour
{
    public TextMeshPro timeTextMesh;

    // Update is called once per frame
    void Update()
    {
        if (timeTextMesh != null)
        {
            timeTextMesh.text = "Time: " + DateTime.Now.ToString("HH:mm:ss");
        }
    }
}
