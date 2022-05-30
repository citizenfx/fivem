namespace Test;

using System.Reflection;
using CitizenFX.Core;
using CitizenFX.Core.Native;
using Moq;
using Xunit;
using static CitizenFX.Core.Native.API;

public class BaseScriptTests
{
    public BaseScriptTests()
    {
        Debug.Reset();
        API.Reset();
    }

    private class TestScript : BaseScript
    {
        [Command("testing:parameterless:unrestricted", Restricted = false)]
        public void UnrestrictedParameterlessCommand()
        {
            Debug.Write("testing:parameterless:unrestricted was successfully called");
        }

        [Command("testing:parameterless:restricted", Restricted = true)]
        public void RestrictedParameterlessCommand()
        {
            Debug.Write("testing:parameterless:restricted was successfully called");
        }

        [Command("testing:parameterless:unrestricted:static", Restricted = false)]
        public static void StaticUnrestrictedParameterlessCommand()
        {
            Debug.Write("testing:parameterless:unrestricted:static was successfully called");
        }

        [Command("testing:parameterless:restricted:static", Restricted = true)]
        public static void StaticRestrictedParameterlessCommand()
        {
            Debug.Write("testing:parameterless:restricted:static was successfully called");
        }

        [Command("testing:player:unrestricted", Restricted = false)]
        public void UnrestrictedPlayerCommand(Player player)
        {
            Debug.Write($"testing:player:unrestricted was successfully called for player {player.Handle}");
        }

        [Command("testing:player:restricted", Restricted = true)]
        public void RestrictedPlayerCommand(Player player)
        {
            Debug.Write($"testing:player:restricted was successfully called for player {player.Handle}");
        }

        [Command("testing:player:unrestricted:static", Restricted = false)]
        public void StaticUnrestrictedPlayerCommand(Player player)
        {
            Debug.Write($"testing:player:unrestricted:static was successfully called for player {player.Handle}");
        }

        [Command("testing:player:restricted:static", Restricted = true)]
        public void StaticRestrictedPlayerCommand(Player player)
        {
            Debug.Write($"testing:player:restricted:static was successfully called for player {player.Handle}");
        }

        [Command("testing:strings:unrestricted", Restricted = false)]
        public void UnrestrictedStringsCommand(string[] args)
        {
            Debug.Write($"testing:strings:unrestricted was successfully called for strings {string.Join(", ", args)}");
        }

        [Command("testing:strings:restricted", Restricted = true)]
        public void RestrictedStringsCommand(string[] args)
        {
            Debug.Write($"testing:strings:restricted was successfully called for strings {string.Join(", ", args)}");
        }

        [Command("testing:strings:unrestricted:static", Restricted = false)]
        public void StaticUnrestrictedStringsCommand(string[] args)
        {
            Debug.Write($"testing:strings:unrestricted:static was successfully called for strings {string.Join(", ", args)}");
        }

        [Command("testing:strings:restricted:static", Restricted = true)]
        public void StaticRestrictedStringsCommand(string[] args)
        {
            Debug.Write($"testing:strings:restricted:static was successfully called for strings {string.Join(", ", args)}");
        }

        [Command("testing:playerstrings:unrestricted", Restricted = false)]
        public void UnrestrictedPlayerStringsCommand(Player player, string[] args)
        {
            Debug.Write($"testing:playerstrings:unrestricted was successfully called for player {player.Handle} with strings {string.Join(", ", args)}");
        }

        [Command("testing:playerstrings:restricted", Restricted = true)]
        public void RestrictedPlayerStringsCommand(Player player, string[] args)
        {
            Debug.Write($"testing:playerstrings:restricted was successfully called for player {player.Handle} with strings {string.Join(", ", args)}");
        }

        [Command("testing:playerstrings:unrestricted:static", Restricted = false)]
        public void StaticUnrestrictedPlayerStringsCommand(Player player, string[] args)
        {
            Debug.Write($"testing:playerstrings:unrestricted:static was successfully called for player {player.Handle} with strings {string.Join(", ", args)}");
        }

        [Command("testing:playerstrings:restricted:static", Restricted = true)]
        public void StaticRestrictedPlayerStringsCommand(Player player, string[] args)
        {
            Debug.Write($"testing:playerstrings:restricted:static was successfully called for player {player.Handle} with strings {string.Join(", ", args)}");
        }

        [Command("testing:legacy:unrestricted", Restricted = false)]
        public void UnrestrictedLegacyCommand(int playerId, List<object> args, string rawCommand)
        {
            Debug.Write($"testing:legacy:unrestricted was successfully called with args {{0}}, {string.Join(", ", args)}, {{1}}", playerId, rawCommand);
        }

        [Command("testing:legacy:restricted", Restricted = true)]
        public void RestrictedLegacyCommand(int playerId, List<object> args, string rawCommand)
        {
            Debug.Write($"testing:legacy:restricted was successfully called with args {{0}}, {string.Join(", ", args)}, {{1}}", playerId, rawCommand);
        }

        [Command("testing:legacy:unrestricted:static", Restricted = false)]
        public void StaticUnrestrictedLegacyCommand(int playerId, List<object> args, string rawCommand)
        {
            Debug.Write($"testing:legacy:unrestricted:static was successfully called with args {{0}}, {string.Join(", ", args)}, {{1}}", playerId, rawCommand);
        }

        [Command("testing:legacy:restricted:static", Restricted = true)]
        public void StaticRestrictedLegacyCommand(int playerId, List<object> args, string rawCommand)
        {
            Debug.Write($"testing:legacy:restricted:static was successfully called with args {{0}}, {string.Join(", ", args)}, {{1}}", playerId, rawCommand);
        }
    }

#if IS_FXSERVER
    [Theory]
    [MemberData(nameof(GetParameterlessCommands))]
    [MemberData(nameof(GetPlayerCommands))]
    [MemberData(nameof(GetStringCommands))]
    [MemberData(nameof(GetPlayerStringsCommands))]
    [MemberData(nameof(GetLegacyCommands))]
    public void InitializeOnAdd_WhenRegisteringACommandOnAServer_ExpectALogToNotBeWritten(string command, bool isRestricted, string logPattern, string executionPattern, int executionPlayerId, List<object> executionParameters, string executionCommand)
    {
        // Given
        var script = new TestScript();

        // When
        script.InitializeOnAdd();

        // Then
        Assert.DoesNotContain(string.Format(logPattern, command), Debug.Outputs);
    }
#else
    [Theory]
    [MemberData(nameof(GetParameterlessCommands))]
    [MemberData(nameof(GetPlayerCommands))]
    [MemberData(nameof(GetStringCommands))]
    [MemberData(nameof(GetPlayerStringsCommands))]
    [MemberData(nameof(GetLegacyCommands))]
    public void InitializeOnAdd_WhenRegisteringACommandOnAClient_ExpectALogToBeWritten(string command, bool isRestricted, string logPattern, string executionPattern, int executionPlayerId, List<object> executionParameters, string executionCommand)
    {
        // Given
        var script = new TestScript();

        // When
        script.InitializeOnAdd();

        // Then
        Assert.Contains($"Registering command {command}\n", Debug.Outputs);
    }
#endif

    [Theory]
    [MemberData(nameof(GetParameterlessCommands))]
    public void InitializeOnAdd_WhenRegisteringAParameterlessCommand_ExpectCallsToTheRegistrationToSucceed(string command, bool isRestricted, string logPattern, string executionPattern, int executionPlayerId, List<object> executionParameters, string executionCommand)
    {
        // Given
        var script = new TestScript();

        // When
        script.InitializeOnAdd();

        // Then
        API.VerifyCommand(command, isRestricted, executionParameters, executionPattern, executionPlayerId, executionCommand, Times.Once());
    }

#if IS_RDR3 || GTA_NY
    [Theory]
    [MemberData(nameof(GetPlayerCommands))]
    public void InitializeOnAdd_WhenRegisteringAPlayerCommand_ExpectCallsToTheRegistrationToThrowAnException(string command, bool isRestricted, string logPattern, string executionPattern, int executionPlayerId, List<object> executionParameters, string executionCommand)
    {
        // Given
        var script = new TestScript();

        // When
        script.InitializeOnAdd();

        // Then
        Assert.Throws<ArgumentException>(() => API.VerifyCommand(command, isRestricted, executionParameters, executionPattern, executionPlayerId, executionCommand, Times.Once()));
    }
#elif IS_FXSERVER
    [Theory]
    [MemberData(nameof(GetPlayerCommands))]
    public void InitializeOnAdd_WhenRegisteringAPlayerCommand_ExpectCommandToBeRegistered(string command, bool isRestricted, string logPattern, string executionPattern, int executionPlayerId, List<object> executionParameters, string executionCommand)
    {
        // Given
        var script = new TestScript();

        // When
        script.InitializeOnAdd();

        // Then
        API.VerifyCommand(command, isRestricted, executionParameters, executionPattern, executionPlayerId, executionCommand, Times.Once());
    }
#else
    [Theory]
    [MemberData(nameof(GetPlayerCommands))]
    public void InitializeOnAdd_WhenRegisteringAPlayerCommand_ExpectCommandToNotBeRegistered(string command, bool isRestricted, string logPattern, string executionPattern, int executionPlayerId, List<object> executionParameters, string executionCommand)
    {
        // Given
        var script = new TestScript();

        // When
        script.InitializeOnAdd();

        // Then
        API.VerifyCommand(command, isRestricted, executionParameters, executionPattern, executionPlayerId, executionCommand, Times.Never());
        Assert.Contains("Client commands with parameter type Player not supported\n", Debug.Outputs);
    }
#endif

    [Theory]
    [MemberData(nameof(GetStringCommands))]
    public void InitializeOnAdd_WhenRegisteringAStringsCommand_ExpectCommandToBeRegistered(string command, bool isRestricted, string logPattern, string executionPattern, int executionPlayerId, List<object> executionParameters, string executionCommand)
    {
        // Given
        var script = new TestScript();

        // When
        script.InitializeOnAdd();

        // Then
        API.VerifyCommand(command, isRestricted, executionParameters, executionPattern, executionPlayerId, executionCommand, Times.Once());
    }

#if IS_RDR3 || GTA_NY
    [Theory]
    [MemberData(nameof(GetPlayerStringsCommands))]
    public void InitializeOnAdd_WhenRegisteringAPlayerStringsCommand_ExpectCallsToTheRegistrationToThrowAnException(string command, bool isRestricted, string logPattern, string executionPattern, int executionPlayerId, List<object> executionParameters, string executionCommand)
    {
        // Given
        var script = new TestScript();

        // When
        script.InitializeOnAdd();

        // Then
        Assert.Throws<System.Reflection.TargetParameterCountException>(() => API.VerifyCommand(command, isRestricted, executionParameters, executionPattern, executionPlayerId, executionCommand, Times.Once()));
    }
#elif IS_FXSERVER
    [Theory]
    [MemberData(nameof(GetPlayerStringsCommands))]
    public void InitializeOnAdd_WhenRegisteringAPlayerStringsCommand_ExpectCommandToBeRegistered(string command, bool isRestricted, string logPattern, string executionPattern, int executionPlayerId, List<object> executionParameters, string executionCommand)
    {
        // Given
        var script = new TestScript();

        // When
        script.InitializeOnAdd();

        // Then
        API.VerifyCommand(command, isRestricted, executionParameters, executionPattern, executionPlayerId, executionCommand, Times.Once());
    }
#else
    [Theory]
    [MemberData(nameof(GetPlayerStringsCommands))]
    public void InitializeOnAdd_WhenRegisteringAPlayerStringsCommand_ExpectCommandToNotBeRegistered(string command, bool isRestricted, string logPattern, string executionPattern, int executionPlayerId, List<object> executionParameters, string executionCommand)
    {
        // Given
        var script = new TestScript();

        // When
        script.InitializeOnAdd();

        // Then
        API.VerifyCommand(command, isRestricted, executionParameters, executionPattern, executionPlayerId, executionCommand, Times.Never());
        Assert.Contains("Client commands with parameter type Player not supported\n", Debug.Outputs);
    }
#endif

    [Theory]
    [MemberData(nameof(GetLegacyCommands))]
    public void InitializeOnAdd_WhenRegisteringALegacyCommand_ExpectCommandToBeRegistered(string command, bool isRestricted, string logPattern, string executionPattern, int executionPlayerId, List<object> executionParameters, string executionCommand)
    {
        // Given
        var script = new TestScript();

        // When
        script.InitializeOnAdd();

        // Then
        API.VerifyCommand(command, isRestricted, executionParameters, executionPattern, executionPlayerId, executionCommand, Times.Once());
    }

    public static IEnumerable<object[]> GetParameterlessCommands()
    {
        yield return new object[]
        {
            /* Command              */ "testing:parameterless:unrestricted",
            /* Restricted           */ false,
            /* Log Pattern          */ "Registering command {0}\n",
            /* Execution Pattern    */ "{0} was successfully called",
            /* Execution Player     */ 1,
            /* Execution Parameters */ new List<object>(),
            /* Execution Command    */ "Test Command 1"
        };

        yield return new object[]
        {
            /* Command              */ "testing:parameterless:restricted",
            /* Restricted           */ true,
            /* Log Pattern          */ "Registering command {0}\n",
            /* Execution Pattern    */ "{0} was successfully called",
            /* Execution Player     */ 2,
            /* Execution Parameters */ new List<object>(),
            /* Execution Command    */ "Test Command 2"
        };

        yield return new object[]
        {
            /* Command              */ "testing:parameterless:unrestricted:static",
            /* Restricted           */ false,
            /* Log Pattern          */ "Registering command {0}\n",
            /* Execution Pattern    */ "{0} was successfully called",
            /* Execution Player     */ 3,
            /* Execution Parameters */ new List<object>(),
            /* Execution Command    */ "Test Command 3"
        };

        yield return new object[]
        {
            /* Command              */ "testing:parameterless:restricted:static",
            /* Restricted           */ true,
            /* Log Pattern          */ "Registering command {0}\n",
            /* Execution Pattern    */ "{0} was successfully called",
            /* Execution Player     */ 4,
            /* Execution Parameters */ new List<object>(),
            /* Execution Command    */ "Test Command 4"
        };
    }

    public static IEnumerable<object[]> GetPlayerCommands()
    {
        var logPattern = "Registering command {0}\n";
        var executionPattern = "{0} was successfully called for player {1}";

        yield return new object[]
        {
            /* Command              */ "testing:player:unrestricted",
            /* Restricted           */ false,
            /* Log Pattern          */ logPattern,
            /* Execution Pattern    */ executionPattern,
            /* Execution Player     */ 1,
            /* Execution Parameters */ new List<object>(),
            /* Execution Command    */ "Test Command 1"
        };

        yield return new object[]
        {
            /* Command              */ "testing:player:restricted",
            /* Restricted           */ true,
            /* Log Pattern          */ logPattern,
            /* Execution Pattern    */ executionPattern,
            /* Execution Player     */ 2,
            /* Execution Parameters */ new List<object>(),
            /* Execution Command    */ "Test Command 2"
        };

        yield return new object[]
        {
            /* Command              */ "testing:player:unrestricted:static",
            /* Restricted           */ false,
            /* Log Pattern          */ logPattern,
            /* Execution Pattern    */ executionPattern,
            /* Execution Player     */ 3,
            /* Execution Parameters */ new List<object>(),
            /* Execution Command    */ "Test Command 3"
        };

        yield return new object[]
        {
            /* Command              */ "testing:player:restricted:static",
            /* Restricted           */ true,
            /* Log Pattern          */ logPattern,
            /* Execution Pattern    */ executionPattern,
            /* Execution Player     */ 4,
            /* Execution Parameters */ new List<object>(),
            /* Execution Command    */ "Test Command 4"
        };
    }

    public static IEnumerable<object[]> GetStringCommands()
    {
        var logPattern = "Registering command {0}\n";
        var executionPattern = "{0} was successfully called for strings {2}";

        yield return new object[]
        {
            /* Command              */ "testing:strings:unrestricted",
            /* Restricted           */ false,
            /* Log Pattern          */ logPattern,
            /* Execution Pattern    */ executionPattern,
            /* Execution Player     */ 1,
            /* Execution Parameters */ new List<object> { "test1", "test2" },
            /* Execution Command    */ "Test Command 1"
        };

        yield return new object[]
        {
            /* Command              */ "testing:strings:restricted",
            /* Restricted           */ true,
            /* Log Pattern          */ logPattern,
            /* Execution Pattern    */ executionPattern,
            /* Execution Player     */ 2,
            /* Execution Parameters */ new List<object> { "test3", "test4", "test 5" },
            /* Execution Command    */ "Test Command 2"
        };

        yield return new object[]
        {
            /* Command              */ "testing:strings:unrestricted:static",
            /* Restricted           */ false,
            /* Log Pattern          */ logPattern,
            /* Execution Pattern    */ executionPattern,
            /* Execution Player     */ 3,
            /* Execution Parameters */ new List<object> { "test6" },
            /* Execution Command    */ "Test Command 3"
        };

        yield return new object[]
        {
            /* Command              */ "testing:strings:restricted:static",
            /* Restricted           */ true,
            /* Log Pattern          */ logPattern,
            /* Execution Pattern    */ executionPattern,
            /* Execution Player     */ 4,
            /* Execution Parameters */ new List<object> { "test7", "test8" },
            /* Execution Command    */ "Test Command 4"
        };
    }

    public static IEnumerable<object[]> GetPlayerStringsCommands()
    {
        var logPattern = "Registering command {0}\n";
        var executionPattern = "{0} was successfully called for player {1} with strings {2}";

        yield return new object[]
        {
            /* Command              */ "testing:playerstrings:unrestricted",
            /* Restricted           */ false,
            /* Log Pattern          */ logPattern,
            /* Execution Pattern    */ executionPattern,
            /* Execution Player     */ 1,
            /* Execution Parameters */ new List<object> { "test1", "test2" },
            /* Execution Command    */ "Test Command 1"
        };

        yield return new object[]
        {
            /* Command              */ "testing:playerstrings:restricted",
            /* Restricted           */ true,
            /* Log Pattern          */ logPattern,
            /* Execution Pattern    */ executionPattern,
            /* Execution Player     */ 2,
            /* Execution Parameters */ new List<object> { "test3", "test4", "test 5" },
            /* Execution Command    */ "Test Command 2"
        };

        yield return new object[]
        {
            /* Command              */ "testing:playerstrings:unrestricted:static",
            /* Restricted           */ false,
            /* Log Pattern          */ logPattern,
            /* Execution Pattern    */ executionPattern,
            /* Execution Player     */ 3,
            /* Execution Parameters */ new List<object> { "test6" },
            /* Execution Command    */ "Test Command 3"
        };

        yield return new object[]
        {
            /* Command              */ "testing:playerstrings:restricted:static",
            /* Restricted           */ true,
            /* Log Pattern          */ logPattern,
            /* Execution Pattern    */ executionPattern,
            /* Execution Player     */ 4,
            /* Execution Parameters */ new List<object> { "test7", "test8" },
            /* Execution Command    */ "Test Command 4"
        };
    }

    public static IEnumerable<object[]> GetLegacyCommands()
    {
        var logPattern = "Registering command {0}\n";
        var executionPattern = "{0} was successfully called with args {1}, {2}, {3}";

        yield return new object[]
        {
            /* Command              */ "testing:legacy:unrestricted",
            /* Restricted           */ false,
            /* Log Pattern          */ logPattern,
            /* Execution Pattern    */ executionPattern,
            /* Execution Player     */ 1,
            /* Execution Parameters */ new List<object> { "test1", "test2" },
            /* Execution Command    */ "Test Command 1"
        };

        yield return new object[]
        {
            /* Command              */ "testing:legacy:restricted",
            /* Restricted           */ true,
            /* Log Pattern          */ logPattern,
            /* Execution Pattern    */ executionPattern,
            /* Execution Player     */ 2,
            /* Execution Parameters */ new List<object> { "test3", "test4", "test 5" },
            /* Execution Command    */ "Test Command 2"
        };

        yield return new object[]
        {
            /* Command              */ "testing:legacy:unrestricted:static",
            /* Restricted           */ false,
            /* Log Pattern          */ logPattern,
            /* Execution Pattern    */ executionPattern,
            /* Execution Player     */ 3,
            /* Execution Parameters */ new List<object> { "test6" },
            /* Execution Command    */ "Test Command 3"
        };

        yield return new object[]
        {
            /* Command              */ "testing:legacy:restricted:static",
            /* Restricted           */ true,
            /* Log Pattern          */ logPattern,
            /* Execution Pattern    */ executionPattern,
            /* Execution Player     */ 4,
            /* Execution Parameters */ new List<object> { "test7", "test8" },
            /* Execution Command    */ "Test Command 4"
        };
    }
}
