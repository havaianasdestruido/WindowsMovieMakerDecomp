using System;
using System.Runtime.InteropServices;

class Program {
    [DllImport("MovieMakerCore.dll", CallingConvention = CallingConvention.Cdecl)]
    public static extern int MovieMakerMain();

    static void Main() {
        int ret = MovieMakerMain();
        Console.WriteLine($"MovieMakerMain returned {ret}");
    }
}
