using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using CitizenFX.Core;
using Lib.AspNetCore.ServerSentEvents;
using Microsoft.AspNetCore.Authorization;
using Microsoft.AspNetCore.Mvc;

using static CitizenFX.Core.Native.API;

namespace FxWebAdmin
{
    public class HomeController : Controller
    {
        private IServerSentEventsService eventsService;

        public HomeController(IServerSentEventsService events)
        {
            this.eventsService = events;
        }

        [HttpGet]
        [AllowAnonymous]
        public async Task<IActionResult> Index()
        {
            var viewData = await HttpServer.QueueTick(() =>
            {
                var resCount = GetResources().Count(a => GetResourceState(a) == "started");

                var data = new IndexData();
                data.Add("Players", $"{GetNumPlayerIndices()}/{GetConvarInt("sv_maxClients", 0)}");
                data.Add("Resources", $"{resCount} running, {GetNumResources()} loaded");

                data.ResourceCount = resCount;

                var numPlayers = GetNumPlayerIndices();
                var totalPing = 0;
                var totalPingCount = 0;

                for (int i = 0; i < numPlayers; i++)
                {
                    var index = GetPlayerFromIndex(i);
                    var ping = GetPlayerPing(index);

                    if (ping > 0)
                    {
                        totalPing += ping;
                        totalPingCount++;
                    }
                }

                if (totalPingCount > 0)
                {
                    data.AverageLatency = (int)((double)totalPing / totalPingCount);
                }

                return data;
            });

            return View(viewData);
        }

        [Authorize(Roles = "webadmin.hi")]
        public async Task<IActionResult> Hi()
        {
            await this.eventsService.SendEventAsync("hi! " + DateTime.UtcNow.ToLongTimeString());

            return View();
        }

        private IEnumerable<string> GetResources()
        {
            for (int i = 0; i < GetNumResources(); i++)
            {
                yield return GetResourceByFindIndex(i);
            }
        }
    }

    public class IndexData
    {
        public int ResourceCount { get; set; }
        public int AverageLatency { get; set; }
        public List<KeyValuePair<string, string>> MetaData { get; } = new List<KeyValuePair<string, string>>();

        public void Add(string key, string value)
        {
            MetaData.Add(new KeyValuePair<string, string>(key, value));
        }
    }
}