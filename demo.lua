local function do_the_thing(label, now, elapsed)
	local time_now     = now();
	local time_elapsed = elapsed(time_now);

	print(label, time_now, time_elapsed);
end

do_the_thing('global',  time_now, time_elapsed);
do_the_thing('table',   time.now, time.elapsed);
do_the_thing('library', libtime.now, libtime.elapsed);
