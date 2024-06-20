using System.Linq.Expressions;
using Moq;
using Xunit;

namespace CitizenFX.Core.Native;

public static class API
{
    public static int PlayerId() => 1;

    public static void RegisterResourceAsEventHandler(string key)
    {
    }

    public static string GetCurrentResourceName()
    {
        return string.Empty;
    }

    public static void RegisterCommand(string command, Action<int, List<object>, string> callback, bool isRestricted)
    {
        _registeredCommands.Add(new RegisteredCommand(command, callback, isRestricted));
    }

    public record RegisteredCommand(string Command, Action<int, List<object>, string>? Callback, bool IsRestricted)
    {
        public override string ToString()
        {
            return $"command[{this.Command}] isRestricted[{this.IsRestricted}]";
        }
    }

    private static List<RegisteredCommand> _registeredCommands = new List<RegisteredCommand>();

    public static void VerifyCommand(string command, bool isRestricted, List<object> parameters, string expectedMessage, int expectedPlayerId, string executionCommand, Times times)
    {
        var registeredCommands = _registeredCommands.Where(x => x.Command == command && x.IsRestricted == isRestricted).ToArray();

        Assert.True(times.Validate(registeredCommands.Length), $"Command expected to be registered {times.ToString()} but was registered {Times.Exactly(registeredCommands.Length)}.\n  Excepected:\n    {new RegisteredCommand(command, default, isRestricted).ToString()}\n  Actual:\n{string.Join("\n    ", _registeredCommands.Select(x => x.ToString()))}");

        for (int i = 0; i < registeredCommands.Length; ++i)
        {
            registeredCommands[i]?.Callback?.Invoke(expectedPlayerId, parameters, executionCommand);

            var temp = string.Format(expectedMessage, command, expectedPlayerId, string.Join(", ", parameters.Select(x => x.ToString())), executionCommand);

            Assert.Contains(temp, Debug.Outputs);
        }
    }

    public static void Reset()
    {
        _registeredCommands = new List<RegisteredCommand>();
    }
}
