name = 'World'
line = ''
first =True
for char in name:
    if first:
        line = char
        first = False
    else:
        line = line + ' ' +char

    print(line)