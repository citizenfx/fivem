using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CitizenFX.Core
{
	[AttributeUsage(AttributeTargets.Method, AllowMultiple = true)]
	public class TickAttribute : Attribute
	{
		public TickAttribute()
		{

		}
	}
}
