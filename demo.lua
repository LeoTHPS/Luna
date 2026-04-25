local function do_the_thing(label, now, elapsed, unix_timestamp)
	local time_now     = now();
	local time_elapsed = elapsed(time_now);

	print(label, unix_timestamp, time_now, time_elapsed);
end

do_the_thing('global',  time_now,    time_elapsed,    unix_timestamp);
do_the_thing('table',   time.now,    time.elapsed,    time.unix_timestamp);
do_the_thing('library', libtime.now, libtime.elapsed, libtime.unix_timestamp);
