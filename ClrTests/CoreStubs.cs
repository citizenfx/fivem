/*
    This contains stubs of different dependencies until we can start including them normally.
*/
namespace CitizenFX.Core;

public class EventHandlerDictionary : Dictionary<string, EventHandlerEntry>
{
}

public class Player
{
    public Player(int id)
    {
        Handle = id;
    }

    public Player(string id)
    {
        Handle = int.Parse(id);
    }

    public int Handle { get; set; }

    public void TriggerEvent(string eventName, params object[] args)
    {
    }

    public void TriggerLatentEvent(string eventName, int bytesPerSecond, params object[] args)
    {
    }
}
public class PlayerList : List<Player>
{
}
public class StateBag
{
    public StateBag(string name)
    {
    }
}
public class EventHandlerEntry
{
    public static EventHandlerEntry operator +(EventHandlerEntry entry, Delegate deleg)
    {
        return entry;
    }

    public static EventHandlerEntry operator -(EventHandlerEntry entry, Delegate deleg)
    {
        return entry;
    }
}
public struct ProfilerScope : IDisposable
{
    public ProfilerScope(object obj)
    {
    }

    public void Dispose()
    {
    }
}
public class CitizenTaskScheduler : TaskScheduler
{
    protected CitizenTaskScheduler()
    {

    }

    protected override IEnumerable<Task>? GetScheduledTasks()
    {
        throw new NotImplementedException();
    }

    protected override void QueueTask(Task task)
    {
        throw new NotImplementedException();
    }

    protected override bool TryExecuteTaskInline(Task task, bool taskWasPreviouslyQueued)
    {
        throw new NotImplementedException();
    }

    public static TaskFactory Factory { get; private set; } = new TaskFactory();
}
public static class Debug
{
    public static List<string> Outputs { get; private set; } = new List<string>();

    public static void Write(string data)
        => Outputs.Add(data);

    public static void Write(string format, params object[] args)
        => Outputs.Add(string.Format(format, args));

    public static void WriteLine()
        => Outputs.Add("\n");

    public static void WriteLine(string data)
        => Outputs.Add($"{data}\n");

    public static void WriteLine(string format, params object[] args)
        => Outputs.Add($"{string.Format(format, args)}\n");

    public static void Reset()
    {
        Outputs = new List<string>();
    }
}

public static class MsgPackSerializer
{
    public static byte[] Serialize(object obj)
    {
        return new byte[0];
    }
}
public enum Hash
{
    TRIGGER_LATENT_SERVER_EVENT_INTERNAL,
    TRIGGER_EVENT_INTERNAL,
    TRIGGER_SERVER_EVENT_INTERNAL,
    TRIGGER_CLIENT_EVENT_INTERNAL,
    TRIGGER_LATENT_CLIENT_EVENT_INTERNAL
}
public static class GameInterface
{
    public static bool SnapshotStackBoundary(out byte[] b)
    {
        b = new byte[0];

        return false;
    }
}
public interface IScriptHost
{
    public void SubmitBoundaryEnd(byte[] b, int length);
}
public static class InternalManager
{
    public static IScriptHost? ScriptHost { get; set; }

    public static void AddScript(BaseScript script)
    {
    }

    public static void RemoveScript(BaseScript script)
    {
    }

    public static void AddDelay(int delay, AsyncCallback callback, string name)
    {
    }
}
public static class Function
{
    unsafe public static void Call(Hash hash, string eventName, byte* serialized, int serializedLength, int bytesPerSecond)
    {
    }
    unsafe public static void Call(Hash hash, string eventName, byte* serialized, int serializedLength)
    {
    }
    unsafe public static void Call(Hash hash, string eventName, string playerId, byte* serialized, int serializedLength, int bytesPerSecond)
    {
    }
    unsafe public static void Call(Hash hash, string eventName, string playerId, byte* serialized, int serializedLength)
    {
    }
}
public class TickAttribute : Attribute
{
}
public class EventHandlerAttribute : Attribute
{
    public string? Name { get; set; }
}
public class ExportDictionary
{
}
