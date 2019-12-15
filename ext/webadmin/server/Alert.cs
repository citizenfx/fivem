using System;
using Newtonsoft.Json;

namespace FxWebAdmin
{
    public class Alert
    {
        [JsonProperty("t")]
        private AlertType type;

        [JsonProperty("v")]
        private string v;

        public AlertType Type => type;

        public string Text => v;

        public Alert(AlertType type, string v)
        {
            this.type = type;
            this.v = v;
        }

        public string Class
        {
            get
            {
                var typeName = "alert-" + Enum.GetName(typeof(AlertType), type).ToLower();

                return $"alert {typeName}";
            }
        }
    }

    public enum AlertType
    {
        Primary,
        Secondary,
        Success,
        Danger,
        Warning,
        Info,
        Light,
        Dark
    }
}