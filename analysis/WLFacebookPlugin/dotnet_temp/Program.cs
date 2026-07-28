using System;
using System.IO;
using System.Collections.Generic;
using System.Reflection;
using System.Linq;

var dllPath = Path.GetFullPath("../../../../undecomp/Photo Gallery/WLFacebookPlugin.dll");
var runtimeDir = Path.GetDirectoryName(typeof(object).Assembly.Location)!;

var paths = new List<string> {
    typeof(object).Assembly.Location,
    Path.Combine(runtimeDir, "System.Runtime.dll"),
    Path.Combine(runtimeDir, "netstandard.dll"),
    dllPath
};

paths.AddRange(Directory.GetFiles(runtimeDir, "*.dll"));

var resolver = new PathAssemblyResolver(paths.Distinct());
using var mlc = new MetadataLoadContext(resolver);
var asm = mlc.LoadFromAssemblyPath(dllPath);

Console.WriteLine("=== ASSEMBLY INFO ===");
Console.WriteLine("FullName: " + asm.FullName);
Console.WriteLine("ImageRuntimeVersion: " + asm.ImageRuntimeVersion);

Console.WriteLine("\n=== REFERENCED ASSEMBLIES ===");
foreach (var refAsm in asm.GetReferencedAssemblies())
    Console.WriteLine("  " + refAsm.Name + " v" + refAsm.Version);

Console.WriteLine("\n=== TYPES ===");
foreach (var type in asm.GetTypes().OrderBy(t => t.FullName))
{
    Console.WriteLine("");
    Console.WriteLine("[" + type.FullName + "]");
    if (type.BaseType != null && type.BaseType.FullName != "System.Object")
        Console.WriteLine("  Base: " + type.BaseType.FullName);
    foreach (var iface in type.GetInterfaces())
        Console.WriteLine("  Implements: " + iface.FullName);
    foreach (var field in type.GetFields(BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Static | BindingFlags.Instance | BindingFlags.DeclaredOnly))
    {
        var val = "";
        try { val = field.IsStatic ? " = " + field.GetValue(null)?.ToString() : ""; } catch { val = " (init-only)"; }
        Console.WriteLine("  Field: " + field.FieldType.Name + " " + field.Name + val);
    }
    foreach (var method in type.GetMethods(BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Static | BindingFlags.Instance | BindingFlags.DeclaredOnly))
    {
        if (method.IsSpecialName) continue;
        var vis = method.IsPublic ? "public" : method.IsPrivate ? "private" : method.IsFamily ? "protected" : "internal";
        var stat = method.IsStatic ? "static " : "";
        var pars = string.Join(", ", method.GetParameters().Select(p => p.ParameterType.Name + " " + p.Name));
        Console.WriteLine("  Method: " + vis + " " + stat + method.ReturnType.Name + " " + method.Name + "(" + pars + ")");
    }
}
