using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net.Http;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Linq;
using Microsoft.Extensions.Caching.Distributed;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;

namespace FxWebAdmin
{
    public class IdentifierHelpers
    {
        private IDistributedCache m_cache;

        public IdentifierHelpers(IDistributedCache cache)
        {
            m_cache = cache;
        }

        public async Task<IEnumerable<IdentifierInfo>> Format(IEnumerable<string> identifiers)
        {
            var tasks = identifiers.Where(a => !string.IsNullOrEmpty(a)).Select(a => Format(a));
            var results = await Task.WhenAll(tasks);

            return results;
        }

        public async Task<IdentifierInfo> Format(string identifier)
        {
            var cacheKey = $".IdentifierInfo:{identifier}";
            var cachedString = await m_cache.GetStringAsync(cacheKey);

            if (cachedString != null)
            {
                return JsonConvert.DeserializeObject<IdentifierInfo>(cachedString);
            }

            var identifierInfoProvider = GetProvider(identifier);

            if (identifierInfoProvider == null)
            {
                identifierInfoProvider = new DummyIdentifierInfoProvider();
            }

            var identifierInfo = await identifierInfoProvider.Lookup(identifier);

            if (identifierInfo == null)
            {
                identifierInfo = await new DummyIdentifierInfoProvider().Lookup(identifier);
            }

            await m_cache.SetStringAsync(cacheKey, JsonConvert.SerializeObject(identifierInfo), new DistributedCacheEntryOptions()
            {
                AbsoluteExpirationRelativeToNow = TimeSpan.FromHours(6)
            });

            return identifierInfo;
        }

        public async Task<string> FormatAvatar(IEnumerable<string> identifiers)
        {
            var tasks = identifiers.Where(a => !string.IsNullOrEmpty(a)).Select(a => Format(a));

            var results = await Task.WhenAll(tasks);

            return results.FirstOrDefault(a => !(a.AvatarUrl ?? "data:").StartsWith("data:"))?.AvatarUrl 
                ?? "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mP8z8DwHwAFBQIAX8jx0gAAAABJRU5ErkJggg==";
        }

        private static IdentifierInfoProvider GetProvider(string identifier)
        {
            var type = identifier.Split(new [] { ':' }, 2)[0];

            switch (type)
            {
                case "discord":
                    return new DiscordIdentifierInfoProvider();
                case "steam":
                    return new SteamIdentifierInfoProvider();
                case "fivem":
                    return new FiveMIdentifierInfoProvider();
            }

            return null;
        }

        private static MaxMind.GeoIP2.DatabaseReader ms_mmdbReader;

        public async Task<string> FormatCountry(string endPoint)
        {
            var cacheKey = $".CountryInfo:{endPoint}";
            var cachedString = await m_cache.GetStringAsync(cacheKey);

            if (cachedString != null)
            {
                return cachedString;
            }

            string cc = "us";
            var geoLitePath = Path.Combine(Startup.RootPath, "GeoLite2-Country.mmdb");

            if (File.Exists(geoLitePath))
            {
                try
                {
                    if (ms_mmdbReader == null)
                    {
                        ms_mmdbReader = new MaxMind.GeoIP2.DatabaseReader(geoLitePath);
                    }

                    var country = ms_mmdbReader.Country(endPoint.Replace("[", "").Replace("]", "").Replace("::ffff:", ""));
                    cc = country.Country.IsoCode.ToLowerInvariant();
                }
                catch {}
            }
            else
            {
                try
                {
                    var response = await ms_httpClient.SendAsync(new HttpRequestMessage(HttpMethod.Get, $"https://freegeoip.app/json/{endPoint.Replace("[", "").Replace("]", "")}"));

                    if (response.IsSuccessStatusCode)
                    {
                        var obj = JObject.Parse(await response.Content.ReadAsStringAsync());
                        cc = obj.Value<string>("country_code").ToLowerInvariant();

                        if (cc == "")
                        {
                            cc = "us";
                        }
                    }
                }
                catch {}
            }

            await m_cache.SetStringAsync(cacheKey, cc, new DistributedCacheEntryOptions()
            {
                AbsoluteExpirationRelativeToNow = TimeSpan.FromHours(6)
            });

            return cc;
        }

        protected static HttpClient ms_httpClient = new HttpClient();
    }

    public abstract class IdentifierInfoProvider
    {
        protected static HttpClient ms_httpClient = new HttpClient();

        public abstract Task<IdentifierInfo> Lookup(string identifier);
    }

    public class DiscordIdentifierInfoProvider : IdentifierInfoProvider
    {
        public override async Task<IdentifierInfo> Lookup(string identifier)
        {
            var userId = identifier.Split(new [] { ':' }, 2)[1];

            try
            {
                var response = await ms_httpClient.SendAsync(new HttpRequestMessage(HttpMethod.Get, $"https://discordapp.com/api/v6/users/{userId}")
                {
                    Headers = {
                        { "Authorization", Encoding.UTF8.GetString(Convert.FromBase64String("Qm90IE5qQXpPVEl5TWpZek1qTTBNRFV5TVRBNS5YVG1ja1EuUDQwcXpwZjlhTXBTSXBDbWl2WFRCOVpua21Z")) }
                    }
                });

                if (response.IsSuccessStatusCode)
                {
                    var obj = JObject.Parse(await response.Content.ReadAsStringAsync());

                    var userInfo = new IdentifierInfo(identifier)
                    {
                        Name = obj.Value<string>("username") + "#" + obj.Value<string>("discriminator"),
                        AvatarUrl = $"https://cdn.discordapp.com/avatars/{userId}/{obj.Value<string>("avatar")}.png",
                        Class = "fab fa-discord",
                        Url = "https://discordapp.com/"
                    };

                    return userInfo;
                }
            }
            catch {}

            return null;
        }
    }

    public class SteamIdentifierInfoProvider : IdentifierInfoProvider
    {
        public override async Task<IdentifierInfo> Lookup(string identifier)
        {
            var userId = long.Parse(identifier.Split(new [] { ':' }, 2)[1],System.Globalization.NumberStyles.AllowHexSpecifier);

            try
            {
                var response = await ms_httpClient.SendAsync(new HttpRequestMessage(HttpMethod.Get, $"https://steamcommunity.com/profiles/{userId}/?xml=1"));

                if (response.IsSuccessStatusCode)
                {
                    var obj = XDocument.Parse(await response.Content.ReadAsStringAsync());

                    var userInfo = new IdentifierInfo(identifier)
                    {
                        Name = obj.Root.Element("steamID").Value,
                        AvatarUrl = obj.Root.Element("avatarMedium").Value,
                        Class = "fab fa-steam",
                        Url = $"https://steamcommunity.com/profiles/{userId}/"
                    };

                    return userInfo;
                }
            }
            catch {}

            return null;
        }
    }

    public class FiveMIdentifierInfoProvider : IdentifierInfoProvider
    {
        public override async Task<IdentifierInfo> Lookup(string identifier)
        {
            var userId = identifier.Split(new [] { ':' }, 2)[1];

            try
            {
                var response = await ms_httpClient.SendAsync(new HttpRequestMessage(HttpMethod.Get, $"https://policy-live.fivem.net/api/getUserInfo/{userId}"));

                if (response.IsSuccessStatusCode)
                {
                    var obj = JObject.Parse(await response.Content.ReadAsStringAsync());
                    string avatarUrl = "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mP8z8DwHwAFBQIAX8jx0gAAAABJRU5ErkJggg==";

                    if (obj.TryGetValue("avatar_template", out var token))
                    {
                        avatarUrl = token.Value<string>().Replace("{size}", "96");
                    }

                    var userInfo = new IdentifierInfo(identifier)
                    {
                        Name = obj.Value<string>("username"),
                        AvatarUrl = avatarUrl,
                        Class = "fa fa-dice-five",
                        Url = $"https://forum.fivem.net/u/{obj.Value<string>("username")}"
                    };

                    return userInfo;
                }
            } catch {}

            return null;
        }
    }

    public class DummyIdentifierInfoProvider : IdentifierInfoProvider
    {
        public override Task<IdentifierInfo> Lookup(string identifier)
        {
            var type = identifier.Split(new [] { ':' }, 2)[0];
            var clss = "fa fa-question";

            switch (type)
            {
                case "xbl":
                    clss = "fab fa-xbox";
                    break;
                case "live":
                    clss = "fab fa-microsoft";
                    break;
                case "discord":
                    clss = "fab fa-discord";
                    break;
                case "steam":
                    clss = "fab fa-steam";
                    break;
                case "license":
                    clss = "fa fa-key";
                    break;
                case "ip":
                    clss = "fa fa-network-wired";
                    break;
            }

            return Task.FromResult(new IdentifierInfo(identifier)
            {
                Name = identifier,
                AvatarUrl = "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mP8z8DwHwAFBQIAX8jx0gAAAABJRU5ErkJggg==",
                Class = clss,
                Url = "#"
            });
        }
    }

    public class IdentifierInfo
    {
        public string Name { get; set; }
        public string Class { get; set; }
        public string Url { get; set; }
        public string AvatarUrl { get; set; }
        public string Identifier { get; }

        public IdentifierInfo(string identifier)
        {
            Identifier = identifier;
        }
    }
}