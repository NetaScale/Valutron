Object subclass: Scheduler [
	"Schedules eco-friendly threads within a native thread."
| (Process)runnable "Head of list of runnable processes"
  (Process)waiting  "Head of list of processes not currently runnable" |

]