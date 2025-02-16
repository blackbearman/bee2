using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bcrypto;

public class Bee2Loader
{
    private const string BinaryPath = "bee2wrap";

    public static void Initialize()
    {
        NativeLibrary.SetDllImportResolver(Assembly.GetExecutingAssembly(), (libraryName, assembly, searchPath) =>
        {
            if (libraryName == BinaryPath)
            {
                var isArm = RuntimeInformation.OSArchitecture == Architecture.Arm64;
                var isOsx = RuntimeInformation.IsOSPlatform(OSPlatform.OSX);
                var isLinux = RuntimeInformation.IsOSPlatform(OSPlatform.Linux);
                var platform = isArm ? "arm64" : "x64";
    
                if (isLinux)
                {
                    return NativeLibrary.Load("runtimes/linux-" + platform 
                        + "/native/" + BinaryPath + ".so", assembly, default);
                }
                else if (isOsx)
                {
                    return NativeLibrary.Load("runtimes/osx-" + platform 
                        + "/native/" + BinaryPath + ".dylib", assembly, default);
                }
    
                return NativeLibrary.Load(BinaryPath, assembly, default);
            }
    
            return default;
        });
    }
}