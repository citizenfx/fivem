using Microsoft.AspNetCore.Mvc.Rendering;
using Microsoft.AspNetCore.Mvc.ViewFeatures;
using Microsoft.AspNetCore.Razor.TagHelpers;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace FxWebAdmin
{
    [HtmlTargetElement("cfx-plugin-outlet")]
    public class PluginOutletTagHelper : TagHelper
    {
        [ViewContext]
        public ViewContext ViewContext { get; set; }

        public override async Task ProcessAsync(TagHelperContext context, TagHelperOutput output)
        {
            if (context.AllAttributes.TryGetAttribute("name", out var nameAttribute))
            {
                output.TagName = null;
                output.Content.SetHtmlContent(
                    await RenderPluginOutlet(
                        nameAttribute.Value.ToString(),
                        context.AllAttributes
                            .Where(a => a.Name != "name")
                            .ToDictionary(a => a.Name, a => a.Value)
                        )
                );
            }
        }

        private async Task<string> RenderPluginOutlet(string name, IDictionary<string, object> attributes)
        {
            return await BaseServer.Self.RunPluginOutlet(name, attributes, ViewContext.HttpContext);
        }
    }
}
