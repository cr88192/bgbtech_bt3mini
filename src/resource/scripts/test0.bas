print "test1"
label1:
print "test2"
x=3
y=4
print "test3 "; 3; " "; 4; " "; x+y

i=0
label2:
print "looptest "; i
i=i+1
if i<10 then goto label2

gosub subtest0

print "postlooptest "; i

end

subtest0:
temp i=-1
for i=0 to 3
print "sublooptest "; i
next i
return
