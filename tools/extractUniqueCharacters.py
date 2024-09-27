def extract_unique_characters_from_txt(file_path, filter_chars):
    unique_chars = set()
    with open(file_path, 'r', encoding='utf - 8') as file:
        for line in file:
            line = line.lstrip()  # 去除行首的空白字符
            if line.startswith('//'):
                continue
            for char in line:
                if char not in filter_chars and (char.isalnum()):
                    unique_chars.add(char)
    return unique_chars


def convert_to_unicode(chars):
    unicode_list = []
    for char in chars:
        unicode_val = hex(ord(char))
        int_val = int(unicode_val, 16)
        unicode_list.append((int_val, unicode_val))
    unicode_list.sort(key = lambda x: x[0])
    return [val for _, val in unicode_list]


file_path = 'cityName.txt'
filter_chars = {'、', ' ', '\n', '：', '。'}
unique_characters = extract_unique_characters_from_txt(file_path, filter_chars)
unicode_chars = convert_to_unicode(unique_characters)

unique_char_list = list(unique_characters)
sorted_unique_char_list = []
for _, unicode_val in sorted([(ord(c), c) for c in unique_char_list], key = lambda x: x[0]):
    sorted_unique_char_list.append(unicode_val)


chunk_size = 8
idx = 0
while idx < len(unicode_chars):
    line_unicode = ', '.join(unicode_chars[idx:idx + chunk_size])
    line_chars = ''.join(sorted_unique_char_list[idx:idx + chunk_size])
    print(f"{line_unicode}, // {line_chars}")
    idx += chunk_size

