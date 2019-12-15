using System;
using CitizenFX.Core;
using CitizenFX.Core.Native;
using Lib.AspNetCore.ServerSentEvents;
using Newtonsoft.Json;

namespace FxWebAdmin
{
    public class ConsoleLog
    {
        private IServerSentEventsService m_eventService;

        public ConsoleLog(IServerSentEventsService events)
        {
            m_eventService = events;

            HttpServer.QueueTick(() => {
                API.RegisterConsoleListener(new Action<string, string>(OnConsoleMessage));
            });
        }

        private void OnConsoleMessage(string channel, string message)
        {
            m_eventService.SendEventAsync(JsonConvert.SerializeObject(message));
        }
    }
}