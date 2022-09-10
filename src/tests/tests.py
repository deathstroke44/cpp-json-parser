import os
os.chdir('..')
relative_path = 'tests/Json files/'
tests = [
    {
        'fileName': 'code-test.json',
        'query': 'skills'
    },
    {
        'fileName': 'code-test.json',
        'query': 'skills.lang'
    },
    {
        'fileName': 'code-test.json',
        'query': 'skills.lang[0]'
    },
    {
        'fileName': 'code-test.json',
        'query': 'skills.lang[0].C'
    },
    {
        'fileName': 'code-test.json',
        'query': 'skills.lang[0].C[1]'
    },
    {
        'fileName': 'code-test.json',
        'query': 'skills.nested1.nested3'
    },
    {
        'fileName': 'code-test.json',
        'query': 'skills.nested1.nested3.key2'
    },
    {
        'fileName': 'large-file.json',
        'query': '[0]'
    },
    {
        'fileName': 'large-file.json',
        'query': '[0].actor'
    },
    {
        'fileName': 'large-file.json',
        'query': '[0].repo'
    },
    {
        'fileName': 'large-file.json',
        'query': '[0].repo.url'
    }
]
os.system('clear')
for test in tests:
    print('Test Case: file name:',test['fileName'], 'query:',test['query'])
    sys_call= 'g++ demo.cpp && ./a.out '+ test['fileName']+' '+ test['query']
    os.system(sys_call)