using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CitizenFX.Core
{
    public sealed class Player
    {
        private int m_playerId;
        private Ped m_ped;
        //private Group m_group;

        public Player(int id)
        {
            m_playerId = id;
        }

        /// <summary>
        /// The player's identifier.
        /// </summary>
        public int ID
        {
            get
            {
                return m_playerId;
            }
        }

        /// <summary>
        /// Returns the player's pedestrian. Obsolete.
        /// </summary>
        [Obsolete("Character is a legacy name dating back to GBHscript; Payne+ renamed this to Ped to match GTA3D conventions. To match modern North engine naming, use Ped.")]
        public Ped Character
        {
            get
            {
                return Ped;
            }
        }

        /// <summary>
        /// Returns the player's pedestrian.
        /// </summary>
        public Ped Ped
        {
            get
            {
                Pointer pedPtr = typeof(int);
                Function.Call(Natives.GET_PLAYER_CHAR, ID, pedPtr);

                int pedId = (int)pedPtr;

                if (m_ped != null)
                {
                    if (m_ped.Handle != pedId)
                    {
                        m_ped.SetHandle(pedId);
                    }
                }
                else
                {
                    m_ped = ObjectCache<Ped>.Get(pedId);
                }

                return m_ped;
            }
        }

        /// <summary>
        /// Returns the player's name.
        /// </summary>
        public string Name
        {
            get
            {
                return Function.Call<string>(Natives.GET_PLAYER_NAME, m_playerId);
            }
        }

        // Model
        // Skin
        // Color
        // Team

        /// <summary>
        /// Returns whether or not the player is ready to participate in the network game.
        /// </summary>
        [Obsolete("Framework naming conventions mandate that properties use PascalCase. Use IsActive instead.")]
        public bool isActive
        {
            get
            {
                return IsActive;
            }
        }

        /// <summary>
        /// Returns whether or not the player is ready to participate in the network game.
        /// </summary>
        public bool IsActive
        {
            get
            {
                return Function.Call<bool>(Natives.IS_NETWORK_PLAYER_ACTIVE, m_playerId);
            }
        }

        /// <summary>
        /// Gets or sets whether or not the player can control their character/pedestrian.
        /// </summary>
        public bool CanControlCharacter
        {
            get
            {
                return Function.Call<bool>(Natives.IS_PLAYER_CONTROL_ON, m_playerId);
            }
            set
            {
                Function.Call(Natives.SET_PLAYER_CONTROL, m_playerId, value);
            }
        }

        /// <summary>
        /// Gets, sets and/or clears the player's wanted level.
        /// </summary>
        public int WantedLevel
        {
            get
            {
                Pointer wantedLevel = typeof(int);
                Function.Call(Natives.STORE_WANTED_LEVEL, m_playerId, wantedLevel);

                return (int)wantedLevel;
            }
            set
            {
                if (value > 0)
                {
                    Function.Call(Natives.ALTER_WANTED_LEVEL, m_playerId, value);
                }
                else
                {
                    Function.Call(Natives.CLEAR_WANTED_LEVEL, m_playerId);
                }

                Function.Call(Natives.APPLY_WANTED_LEVEL_CHANGE_NOW, m_playerId);
            }
        }

        // Money

        // LastVehicle
    
        /// <summary>
        /// Returns whether or not the player is local to this machine.
        /// </summary>
        [Obsolete("Framework naming conventions, use IsLocalPlayer instead.")]
        public bool isLocalPlayer
        {
            get
            {
                return IsLocalPlayer;
            }
        }

        /// <summary>
        /// Gets whether or not the player is local to this machine.
        /// </summary>
        public bool IsLocalPlayer
        {
            get
            {
                return (m_playerId == Function.Call<int>(Natives.GET_PLAYER_ID));
            }
        }

        /// <summary>
        /// Gets whether or not the player is ready to be acted upon by script.
        /// </summary>
        public bool IsPlaying
        {
            get
            {
                return Function.Call<bool>(Natives.IS_PLAYER_PLAYING, m_playerId);
            }
        }

        /// <summary>
        /// Gets whether or not the player is currently pressing the Horn control for their vehicle.
        /// </summary>
        public bool IsPressingHorn
        {
            get
            {
                return Function.Call<bool>(Natives.IS_PLAYER_PRESSING_HORN, m_playerId);
            }
        }

        /// <summary>
        /// Sets a value indicating to artificial intelligence to ignore this player.
        /// </summary>
        public bool IgnoredByEveryone
        {
            set
            {
                Function.Call(Natives.SET_EVERYONE_IGNORE_PLAYER, m_playerId, value);
            }
        }

        /// <summary>
        /// Sets a value indicating whether or not the player can control their ragdolls.
        /// </summary>
        public bool CanControlRagdoll
        {
            set
            {
                Function.Call(Natives.GIVE_PLAYER_RAGDOLL_CONTROL, m_playerId, value);
            }
        }

        [Obsolete("Irrelevant in network games.")]
        public bool NeverGetsTired
        {
            set
            {

            }
        }

        /// <summary>
        /// Determine if the player is targeting a particular ped.
        /// </summary>
        /// <param name="ped">The ped to test against/</param>
        /// <returns>A boolean value.</returns>
        [Obsolete("Both framework naming conventions mismatch as well as that this is a British name; use IsTargeting instead.")]
        public bool isTargetting(Ped ped)
        {
            return IsTargeting(ped);
        }

        /// <summary>
        /// Determine if the player is targeting a particular ped.
        /// </summary>
        /// <param name="ped">The ped to test against/</param>
        /// <returns>A boolean value.</returns>
        public bool IsTargeting(Ped ped)
        {
            return Function.Call<bool>(Natives.IS_PLAYER_FREE_AIMING_AT_CHAR, m_playerId, ped.Handle);
        }

        // GetTargetedPed
        // ^ needs to have Lua equivalent too for fairness

        // SetComponentVisibility
        // ActivateMultiplayerSkin

        /// <summary>
        /// Teleports the local player to a specific coordinate, loading the scene while they go.
        /// </summary>
        /// <param name="position">The position to teleport to.</param>
        public void TeleportTo(Vector3 position)
        {
            if (!IsLocalPlayer)
            {
                throw new InvalidOperationException("Can not teleport remote players.");
            }

            var ped = Ped;

            // TODO: actually teleport
        }

        public void TeleportTo(Vector2 position)
        {
            TeleportTo(new Vector3(position, 0.0f));
        }

        public static bool operator==(Player left, Player right)
        {
            if (object.ReferenceEquals(left, null))
            {
                return object.ReferenceEquals(right, null);
            }

            if (object.ReferenceEquals(right, null))
            {
                return false;
            }

            return left.ID == right.ID;
        }

        public static bool operator!=(Player left, Player right)
        {
            return !(left == right);
        }

        // maybe: player/ped comparison?
    }
}
