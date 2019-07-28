using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;

using CitizenFX.Core;
using CitizenFX.Core.Native;
using Microsoft.AspNetCore.Hosting;
using Microsoft.AspNetCore.Hosting.Server;
using Microsoft.AspNetCore.Mvc.ApplicationParts;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Logging;
using static CitizenFX.Core.Native.API;

namespace FxWebAdmin
{
    public class BaseServer : BaseScript
    {
        public static BaseServer Self { get; private set; }

        public IEnumerable<Player> ExternalPlayers => Players.ToArray(); // ToArray so that it doesn't break if used async

        public BaseServer()
        {
            Tick += FirstTick;
            Self = this;
        }

        private async Task FirstTick()
        {
            Tick -= FirstTick;

            Environment.SetEnvironmentVariable("MONO_MANAGED_WATCHER", "disabled");

            // screw TLS certs especially as they require manually deploying a root list.. really?
            ServicePointManager.ServerCertificateValidationCallback += 
                (sender, cert, chain, sslPolicyErrors) => true;

            var host = new WebHostBuilder()
                .ConfigureServices(services => {
                    services.AddSingleton<IServer, HttpServer>();
                    services.AddSingleton<ConsoleLog>();
                })
                .ConfigureLogging(l =>
                {
                    //l.SetMinimumLevel(LogLevel.Trace);
                    //l.AddConsole();
                })
                .UseContentRoot(GetResourcePath(GetCurrentResourceName()))
                .UseStartup<Startup>()
                .Build();

            host.Start();

            Encoding.RegisterProvider(CodePagesEncodingProvider.Instance);

            // try downloading maxmind
            var geoLitePath = Path.Combine(Startup.RootPath, "GeoLite2-Country.mmdb");

            if (!File.Exists(geoLitePath))
            {
                await Task.Run(async () =>
                {
                    try
                    {
                        var httpClient = new HttpClient();
                        var httpResponse = await httpClient.GetAsync("https://geolite.maxmind.com/download/geoip/database/GeoLite2-Country.tar.gz", HttpCompletionOption.ResponseHeadersRead);

                        if (httpResponse.IsSuccessStatusCode)
                        {
                            using (var stream = await httpResponse.Content.ReadAsStreamAsync())
                            {
                                using (var reader = SharpCompress.Readers.ReaderFactory.Open(stream))
                                {
                                    while (reader.MoveToNextEntry())
                                    {
                                        if (reader.Entry.Key.EndsWith(".mmdb"))
                                        {
                                            using (var file = File.Open(geoLitePath, FileMode.Create, FileAccess.Write))
                                            {
                                                reader.WriteEntryTo(file);

                                                Debug.WriteLine($"Saved GeoLite2-Country to {geoLitePath}.");

                                                break;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    catch {}
                });
            }
        }
    }
}
