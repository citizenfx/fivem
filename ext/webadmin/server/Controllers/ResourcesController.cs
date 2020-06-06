using System.Collections.Generic;
using System.Threading.Tasks;
using Microsoft.AspNetCore.Authorization;
using Microsoft.AspNetCore.Http;
using Microsoft.AspNetCore.Mvc;

using static CitizenFX.Core.Native.API;

namespace FxWebAdmin
{
    public class ResourcesController : Controller
    {
        [Authorize(Roles = "command.start,command.restart,command.stop,webadmin.resources.view")]
        [HttpGet]
        public IActionResult List()
        {
            return View(new ResourcesData());
        }

        [Authorize(Roles = "command.refresh,webadmin.resources.refresh")]
        [HttpPost]
        public async Task<IActionResult> Refresh()
        {
            await HttpServer.QueueTick(() =>
            {
                // TODO: access control for this(!)
                ExecuteCommand("refresh");
            });

            return RedirectToAction("List");
        }

        [Authorize(Roles = "command.start,webadmin.resources.start")]
        [HttpPost]
        public async Task<IActionResult> Start([FromForm] string resource)
        {
            var success = await HttpServer.QueueTick(() =>
            {
                return StartResource(resource);
            });

            HttpContext.Session.Set("alert", (success) ?
                new Alert(AlertType.Success, $"{resource} has been successfully started.") :
                new Alert(AlertType.Danger, $"{resource} failed to start immediately. See the server console for details."));

            return RedirectToAction("List");
        }

        [Authorize(Roles = "command.stop,webadmin.resources.stop")]
        [HttpPost]
        public async Task<IActionResult> Stop([FromForm] string resource)
        {
            if (resource != "webadmin")
            {
                var success = await HttpServer.QueueTick(() =>
                {
                    return StopResource(resource);
                });

                HttpContext.Session.Set("alert", (success) ?
                    new Alert(AlertType.Success, $"{resource} has been successfully stopped.") :
                    new Alert(AlertType.Danger, $"{resource} failed to stop. See the server console for details."));
            }

            return RedirectToAction("List");
        }

        [Authorize(Roles = "command.restart,webadmin.resources.restart")]
        [HttpPost]
        public async Task<IActionResult> Restart([FromForm] string resource)
        {
            if (resource != "webadmin")
            {
                var success = await HttpServer.QueueTick(() =>
                {
                    if (StopResource(resource))
                    {
                        return StartResource(resource);
                    }

                    return false;
                });

                HttpContext.Session.Set("alert", (success) ?
                    new Alert(AlertType.Success, $"{resource} has been successfully restarted.") :
                    new Alert(AlertType.Danger, $"{resource} failed to restart immediately. See the server console for details."));
            }

            return RedirectToAction("List");
        }
    }

    public class ResourcesData
    {
        public IEnumerable<string> GetResources()
        {
            for (int i = 0; i < GetNumResources(); i++)
            {
                var resourceName = GetResourceByFindIndex(i);

                if (resourceName == "webadmin" || resourceName == "_cfx_internal")
                {
                    continue;
                }

                yield return resourceName;
            }
        }

        public IEnumerable<string> GetMetadata(string resource, string kind)
        {
            for (int i = 0; i < GetNumResourceMetadata(resource, kind); i++)
            {
                yield return GetResourceMetadata(resource, kind, i);
            }
        }
    }
}