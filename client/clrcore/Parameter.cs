using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CitizenFX.Core
{
    public struct Parameter
    {
        internal enum ParameterType
        {
            None,
            Integer,
            Float,
            Vector3,
            String,
            IntPointer,
            FloatPointer
        }

        private object m_value;
        private ParameterType m_type;

        public Parameter(int integer)
        {
            m_value = integer;
            m_type = ParameterType.Integer;
        }

        public static implicit operator Parameter(int integer)
        {
            return new Parameter(integer);
        }

        public Parameter(float number)
        {
            m_value = number;
            m_type = ParameterType.Float;
        }

        public static implicit operator Parameter(float number)
        {
            return new Parameter(number);
        }

        public Parameter(Pointer value)
        {
            m_value = value;
            m_type = value.ParameterType;            
        }

        public static implicit operator Parameter(Pointer value)
        {
            return new Parameter(value);
        }

        public Parameter(string str)
        {
            m_value = str;
            m_type = ParameterType.String;
        }

        public static implicit operator Parameter(string str)
        {
            return new Parameter(str);
        }

        public Parameter(bool boolean)
        {
            m_value = boolean ? 1 : 0;
            m_type = ParameterType.Integer;
        }

        public static implicit operator Parameter(bool boolean)
        {
            return new Parameter(boolean);
        }

        internal ParameterType Type
        {
            get
            {
                return m_type;
            }
        }

        internal object Value
        {
            get
            {
                return m_value;
            }
        }
    }

    public class Pointer
    {
        private object m_value;
        private Type m_type;

        public Pointer(Type type)
        {
            m_type = type;
        }

        public Pointer(int value)
        {
            m_type = typeof(int);
            m_value = value;
        }

        public Pointer(float value)
        {
            m_type = typeof(float);
            m_value = value;
        }

        public static implicit operator Pointer(Type type)
        {
            return new Pointer(type);
        }

        public static implicit operator Pointer(int value)
        {
            return new Pointer(value);
        }

        public static implicit operator Pointer(float value)
        {
            return new Pointer(value);
        }

        internal object Value
        {
            get
            {
                return m_value;
            }
        }

        public static explicit operator int(Pointer pointer)
        {
            return pointer.GetValue<int>();
        }

        public static explicit operator float(Pointer pointer)
        {
            return pointer.GetValue<float>();
        }

        public T GetValue<T>()
        {
            return (T)m_value;
        }

        internal void SetValue(object value)
        {
            if (m_type == typeof(int) || m_type == typeof(float))
            {
                m_value = value;
            }
        }

        internal Parameter.ParameterType ParameterType
        {
            get
            {
                if (m_type == typeof(float))
                {
                    return Parameter.ParameterType.FloatPointer;
                }
                else
                {
                    return Parameter.ParameterType.IntPointer;
                }
            }
        }
    }
}
