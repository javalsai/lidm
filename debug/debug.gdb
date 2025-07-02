# this just meant to run in another term, is probably better to attach to another already running window
set inferior-tty /dev/pts/7
set pagination off

break *(read_press + 62)
define fake_read
    set *((char*) $rsi) = $al
    set $rax = 1
    jump *(read_press + 67)
end

break *(read_press + 108)
define fake_magic_cont
    set $eax = 1
    jump *(read_press + 113)
end
define fake_magic_end
    set $eax = 0
    jump *(read_press + 113)
end

run
set $al = 0x1b
fake_read
fake_magic_cont
set $al = '['
fake_read
fake_magic_cont
set $al = 'D'
fake_read
fake_magic_end

set $al = 'p'
fake_read
fake_magic_end

set $al = 'a'
fake_read
fake_magic_end

set $al = 's'
fake_read
fake_magic_end

set $al = 's'
fake_read
fake_magic_end

set $al = 'w'
fake_read
fake_magic_end

set $al = 'd'
fake_read
fake_magic_end

set $al = '\n'
fake_read
fake_magic_end
