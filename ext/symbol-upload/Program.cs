using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using Microsoft.SymbolUploader;

namespace CitizenFX.BuildTools.SymbolUpload
{
    class Program
    {
        static void Main(string[] args)
        {
            if (args.Length == 0)
            {
                Console.WriteLine("\r\nsymbol-upload -oOUTPATH <files>");
                return;
            }

            var files = new List<string>();
            string outPath = null;

            foreach (var arg in args)
            {
                if (arg.StartsWith("-o"))
                {
                    outPath = arg.Substring(2);
                    continue;
                }

                files.Add(arg);
            }

            if (outPath == null)
            {
                Console.WriteLine("No output path specified.");
                return;
            }

            var inputFiles = files.SelectMany(file =>
            {
                string directoryName = Path.GetDirectoryName(file);
                string fileName = Path.GetFileName(file);
                return Directory.EnumerateFiles(string.IsNullOrWhiteSpace(directoryName) ? "." : directoryName, fileName, SearchOption.TopDirectoryOnly);
            });

            if (!inputFiles.Any())
            {
                Console.WriteLine("Input files not found.");
                return;
            }

            var po = new PublishOperation(new Tracer()
            {
                Enabled = true
            });

            var publishFiles = po.GetPublishFileInfo(inputFiles, false);

            foreach (var file in publishFiles)
            {
                var outName = Path.Combine(outPath, file.Index);
                Directory.CreateDirectory(Path.GetDirectoryName(outName));
                File.Copy(file.FileName, outName, true);
            }
        }
    }
}
