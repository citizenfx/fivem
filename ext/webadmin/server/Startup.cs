using System;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Security.Cryptography;
using System.Threading.Tasks;
using Lib.AspNetCore.ServerSentEvents;
using Microsoft.AspNetCore.Authentication;
using Microsoft.AspNetCore.Authentication.Cookies;
using Microsoft.AspNetCore.Authorization;
using Microsoft.AspNetCore.Builder;
using Microsoft.AspNetCore.Http;
using Microsoft.AspNetCore.Mvc;
using Microsoft.AspNetCore.Mvc.ApplicationParts;
using Microsoft.AspNetCore.Mvc.Authorization;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.FileProviders;

using static CitizenFX.Core.Native.API;

namespace FxWebAdmin
{
    public class Startup
    {
        public static string RootPath = GetResourcePath(GetCurrentResourceName());

        public void ConfigureServices(IServiceCollection services)
        {
            services.AddDistributedMemoryCache();

            services.AddSession(options =>
            {
                options.IdleTimeout = TimeSpan.FromHours(1);
                options.Cookie.IsEssential = true;
                options.Cookie.Name = ".FxWebAdmin.Session";
            });

            services.AddAuthentication(CookieAuthenticationDefaults.AuthenticationScheme)
                .AddCookie(options => {
                    options.LoginPath = "/Auth/Login";
                    options.LogoutPath = "/Auth/Logout";

                    options.Events.OnValidatePrincipal = async context =>
                    {
                        if (context.Principal.HasClaim(a => a.Type == "cfx:rcon_password"))
                        {
                            var claim = context.Principal.FindFirst("cfx:rcon_password");

                            if (GetConvar("rcon_password", "") != claim.Value)
                            {
                                context.RejectPrincipal();

                                await context.HttpContext.SignOutAsync(CookieAuthenticationDefaults.AuthenticationScheme);
                            }
                        }
                    };
                })
                .AddSteam("steam", options => {
                    options.ApplicationKey = "04246554B51E8C14ED537C55A4BA4CD7";
                })
                .AddRemoteScheme<Authentication.DiscourseAuthenticationOptions, Authentication.DiscourseAuthenticationHandler>
                    ("discourse", "Discourse", options => {
                        var clientId = "12345678901234567890123456789012";

                        string BuildRandomString(int bytes)
                        {
                            var byteArray = new byte[bytes];

                            using (var rng = RandomNumberGenerator.Create())
                            {
                                rng.GetBytes(byteArray);
                            }

                            return string.Concat(byteArray.Select(a => $"{a:X2}"));
                        }

                        var clientIdPath = Path.Combine(Startup.RootPath, "discourse_client_id");

                        try
                        {
                            if (File.Exists(clientIdPath))
                            {
                                clientId = File.ReadAllText(clientIdPath);
                            }
                            else
                            {
                                clientId = BuildRandomString(16);

                                File.WriteAllText(clientIdPath, clientId);
                            }
                        }
                        catch {}

                        using (var rsa = RSA.Create())
                        {
                            options.RSAKey = rsa.ExportParameters(true);
                            options.ClientId = clientId;
                            options.ApplicationName = $"FXServer on {GetConvar("sv_hostname", "")}";
                            options.DiscourseUri = new Uri("https://forum.fivem.net/");
                        }
                    });

            services.AddTransient<IClaimsTransformation, FxClaimsTransformer>();
            services.AddSingleton<IdentifierHelpers, IdentifierHelpers>();

            services.AddMvc(service => {
                var policy = new AuthorizationPolicyBuilder()
                    .AddAuthenticationSchemes("discourse", "steam", CookieAuthenticationDefaults.AuthenticationScheme)
                    .RequireAuthenticatedUser()
                    .Build();
                    
                service.Filters.Add(new AuthorizeFilter(policy));
            })
            .ConfigureApplicationPartManager(apm => {
                var a = Assembly.Load("FxWebAdmin.net.Views");
                
                var assemblies = new[] { a };
                foreach (var assembly in assemblies)
                {
                    var partFactory = ApplicationPartFactory.GetApplicationPartFactory(assembly);
                    foreach (var part in partFactory.GetApplicationParts(assembly))
                    {
                        apm.ApplicationParts.Add(part);
                    }
                }
            })
            .SetCompatibilityVersion(CompatibilityVersion.Version_2_2);

            services.AddServerSentEvents();
        }

        public void Configure(IApplicationBuilder app)
        {
            app.UseSession();

            app.UseAuthentication();

            app.MapServerSentEvents("/events", new ServerSentEventsOptions()
            {
                Authorization = new ServerSentEventsAuthorization { Roles = "webadmin.console.read" }
            });

            app.UseMvc(routes => {
                routes.MapRoute(
                    name: "default",
                    template: "{controller=Home}/{action=Index}/{id?}"
                );
            });

            app.UseStaticFiles(new StaticFileOptions()
            {
                FileProvider = new PhysicalFileProvider(Path.Combine(GetResourcePath(GetCurrentResourceName()), "wwwroot"))
            });
        }
    }
}