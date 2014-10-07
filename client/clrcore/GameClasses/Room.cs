using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CitizenFX.Core
{
    public sealed class Room
    {
        private int pRoomKey;
        private int pInteriorID;

        internal Room(int RoomKey, int InteriorID)
        {
            pRoomKey = RoomKey;
            pInteriorID = InteriorID;
        }
        internal int RoomKey
        {
            get
            {
                return pRoomKey;
            }
        }
        internal int InteriorID
        {
            get
            {
                return pInteriorID;
            }
        }

        public static Room FromString(string StringID)
        {
            if (StringID == "") return new Room(0, 0);
            string[] vals = StringID.Trim().Split('_');
            if ((vals.Length != 3) || (vals[0] != "R")) return new Room(0, 0);
            return new Room(int.Parse(vals[1], System.Globalization.NumberStyles.HexNumber), int.Parse(vals[2], System.Globalization.NumberStyles.HexNumber));
        }

        public static bool operator ==(Room left, Room right)
        {
            return ((left.RoomKey == right.RoomKey) && (left.InteriorID == right.InteriorID));
        }
        public static bool operator !=(Room left, Room right)
        {
            return !(left == right);
        }
    }
}
