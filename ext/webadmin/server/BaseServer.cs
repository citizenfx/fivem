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
using Microsoft.AspNetCore.Http;
using Microsoft.AspNetCore.Mvc;
using Microsoft.AspNetCore.Mvc.ApplicationParts;
using Microsoft.AspNetCore.Mvc.Routing;
using Microsoft.AspNetCore.Routing;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Logging;
using static CitizenFX.Core.Native.API;

namespace FxWebAdmin
{
    public class BaseServer : BaseScript
    {
        private readonly Dictionary<string, List<Tuple<Func<IDictionary<string, object>, string>, string>>> m_pluginOutlets =
            new Dictionary<string, List<Tuple<Func<IDictionary<string, object>, string>, string>>>();

        private readonly Dictionary<string, Tuple<Func<IDictionary<string, object>, string>, string>> m_pluginPages =
            new Dictionary<string, Tuple<Func<IDictionary<string, object>, string>, string>>();

        public static BaseServer Self { get; private set; }

        public IEnumerable<Player> ExternalPlayers => Players.ToArray(); // ToArray so that it doesn't break if used async

        public HttpContext CurrentContext { get; private set; }

        public BaseServer()
        {
            Tick += FirstTick;
            Self = this;

            Exports.Add("registerPluginOutlet", new Action<string, CallbackDelegate>(this.RegisterPluginOutlet));
            Exports.Add("registerPluginPage", new Action<string, CallbackDelegate>(this.RegisterPluginPage));
            Exports.Add("isInRole", new Func<string, bool>(this.IsInRole));
            Exports.Add("getPluginUrl", new Func<string, IDictionary<string, object>, string>(this.GetPluginUrl));
        }

        private string GetPluginUrl(string name, IDictionary<string, object> attributes = null)
        {
            var attrs = attributes ?? new Dictionary<string, object>();
            attrs["name"] = name;

            var linkGenerator = CurrentContext.RequestServices.GetService<LinkGenerator>();
            return linkGenerator.GetPathByAction(CurrentContext, "Page", "Plugin", attrs);
        }

        private bool IsInRole(string role)
        {
            return CurrentContext?.User?.IsInRole(role) ?? false;
        }

        private void RegisterPluginOutlet(string name, CallbackDelegate callback)
        {
            var resourceName = GetInvokingResource();

            if (!m_pluginOutlets.TryGetValue(name, out var list))
            {
                list = new List<Tuple<Func<IDictionary<string, object>, string>, string>>();
                m_pluginOutlets[name] = list;
            }

            list.Add(
                Tuple.Create<Func<IDictionary<string, object>, string>, string>(
                    attributes => callback.Invoke(attributes)?.ToString() ?? "",
                    resourceName
                )
            );
        }

        private void RegisterPluginPage(string name, CallbackDelegate callback)
        {
            var resourceName = GetInvokingResource();
            
            m_pluginPages[$"{name}"] =
                Tuple.Create<Func<IDictionary<string, object>, string>, string>(
                    attributes =>
                    {
                        var invokeResult = callback.Invoke(attributes);
                        string s;

                        if (invokeResult is Task<object> t)
                        {
                            // TODO: lol blocking?
                            s = (t.Result)?.ToString() ?? "";
                        }
                        else
                        {
                            s = invokeResult?.ToString() ?? "";
                        }

                        return s;
                    },
                    resourceName
                );
        }

        [EventHandler("onResourceStop")]
        public void OnResourceStop(string resourceName)
        {
            foreach (var outlet in m_pluginOutlets.Values)
            {
                foreach (var entry in outlet.ToArray())
                {
                    if (entry.Item2 == resourceName)
                    {
                        outlet.Remove(entry);
                    }
                }
            }
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

        internal async Task<string> RunPluginController(string name, IDictionary<string, object> attributes, HttpContext context)
        {
            if (m_pluginPages.TryGetValue(name, out var page))
            {
                return await HttpServer.QueueTick(() =>
                {
                    CurrentContext = context;
                    var s = page.Item1(attributes);
                    CurrentContext = null;

                    return s;
                });
            }

            return "";
        } 

        internal async Task<string> RunPluginOutlet(string name, IDictionary<string, object> attributes, HttpContext context)
        {
            if (m_pluginOutlets.TryGetValue(name, out var outletList))
            {
                return await HttpServer.QueueTick(() =>
                {
                    CurrentContext = context;
                    var s = string.Join("", outletList.Select(cb => cb.Item1(attributes)));
                    CurrentContext = null;

                    return s;
                });
            }

            return "";
        }
    }
}
