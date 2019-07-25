using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Reflection;
using System.Threading.Tasks;

using CitizenFX.Core;
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

        private Task FirstTick()
        {
            Tick -= FirstTick;

            Environment.SetEnvironmentVariable("MONO_MANAGED_WATCHER", "disabled");

            // screw TLS certs especially as they require manually deploying a root list.. really?
            ServicePointManager.ServerCertificateValidationCallback += 
                (sender, cert, chain, sslPolicyErrors) => true;

            var host = new WebHostBuilder()
                .ConfigureServices(services => {
                    services.AddSingleton<IServer, HttpServer>();
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

            return Task.CompletedTask;
        }
    }
}
