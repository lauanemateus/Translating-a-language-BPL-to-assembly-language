// ####################################################################
// Alunos envolvidos no trabalho
// Lauane Mateus Oliveira de Moraes - 202103747
// Luis Miguel Gouveia Machado - 202103755
// Nelsi de Sousa Barbosa Junior - 202103764
// ####################################################################

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINESZ 256

// Remove o '\n' do fim da linha
void remove_newline(char *ptr)
{
    while (*ptr)
    {
        if (*ptr == '\n')
            *ptr = 0;
        else
            ptr++;
    }
}

void get_array_pos_i(int i);
int operation(char c);
int bloco_def_variaveis_e_alocacao_pilha(int posicao_variaveis[], char line[],
                                         int *tampilha, int qtparam);
void atribuicao_de_variaveis(int posicao_variaveis[], char line[]);
int definicoes_de_variaveis(int posicao_variaveis[], char line[], int *tampilha,
                            int qtparam);
void set_array(int posicao_variaveis[], char line[]);
void get_array(int posicao_variaveis[], char line[]);
void bloco_def(int posicao_variaveis[], char line[], int *tampilha);
void call(int posicao_variaveis[], char line[], int qtd_registradores,
          int qtparam, int tampilha);
void condicional(int posicao_variaveis[], char line[], int *qt_if,
                 int qtd_registradores, int qtparam, int tampilha);
void recupera_parametros(int pos, int qtparam);
void salva_parametros(int pos, int qtparam);
void retorna(int posicao_variaveis[], char line[], int qtd_registradores,
             int qtparam, int tampilha);
void divisao();
void end_code(int tampilha, int qtd_registradores, int qtparam);

void get_array_pos_i(int i)
{
    // dado que o enderço de um array (&v) está em %r8, fazemos o enderço de &v[i]
    // estar em %r8
    printf("    movq $%d, %%r9\n", i);
    printf("    imulq $4, %%r9\n");
    printf("    addq %%r9, %%r8\n");
}

void salva_parametros(int pos, int qtparam)
{
    if (qtparam >= 1)
    {
        printf("    movq %%rdi, %d(%%rbp)\n", pos - 8);
        pos -= 8;
    }
    if (qtparam >= 2)
    {
        printf("    movq %%rsi, %d(%%rbp)\n", pos - 8);
        pos -= 8;
    }
    if (qtparam >= 3)
    {
        printf("    movq %%rdx, %d(%%rbp)\n", pos - 8);
        pos -= 8;
    }
    if (qtparam)
        printf("\n");
}

void recupera_parametros(int pos, int qtparam)
{
    if (qtparam >= 1)
    {
        printf("    movq %d(%%rbp),%%rdi\n", pos - 8);
        pos -= 8;
    }
    if (qtparam >= 2)
    {
        printf("    movq %d(%%rbp),%%rsi\n", pos - 8);
        pos -= 8;
    }
    if (qtparam >= 3)
    {
        printf("    movq %d(%%rbp),%%rdx\n", pos - 8);
        pos -= 8;
    }
    if (qtparam)
        printf("\n");
}

int operation(char c)
{
    if (c == '+')
        printf("    addl ");
    if (c == '-')
        printf("    subl ");
    if (c == '*')
        printf("    imull ");
    if (c != '/')
        return 1;
    return 0;
}

void divisao()
{
    // salvando %rax e %rdx para recuperar depois
    printf("    movq %%rax, %%r10\n");
    printf("    movq %%rdx, %%r11\n");
    // consideramos que o dividendo sempre está salvo no %r8d e o divisor no %r9d
    printf("    movl %%r8d, %%eax\n    cltd\n    idivl %%r9d\n");
    printf("    movl %%eax, %%r8d\n"); // movendo o resultado para %r8d
    // recuperando %rax e %rdx
    printf("    movq %%r10, %%rax\n");
    printf("    movq %%r11, %%rdx\n");
}

void comparacoes_if(char c1, char c2, int qt_if)
{
    if (c1 == 'e' && c2 == 'q')
        printf("    jne end_if_%d\n", qt_if);
    if (c1 == 'n' && c2 == 'e')
        printf("    je end_if_%d\n", qt_if);
    if (c1 == 'l' && c2 == 't')
        printf("    jge end_if_%d\n", qt_if);
    if (c1 == 'l' && c2 == 'e')
        printf("    jg end_if_%d\n", qt_if);
    if (c1 == 'g' && c2 == 't')
        printf("    jle end_if_%d\n", qt_if);
    if (c1 == 'g' && c2 == 'e')
        printf("    jl end_if_%d\n", qt_if);
}

void condicional(int posicao_variaveis[], char line[], int *qt_if,
                 int qtd_registradores, int qtparam, int tampilha)
{
    int id1, id2, tipo;
    char c1, c2, a1, b1, a2, b2;

    tipo = sscanf(line, "if %c%c%d %c%c %c%c%d", &a1, &b1, &id1, &c1, &c2, &a2,
                  &b2, &id2);

    if (tipo == 8)
    {
        (*qt_if)++; // incrementa na quantidade de if's
        printf("    # ----------------------------------------------\n");
        printf("    if_%d:\n\n", (*qt_if));

        // variavel inteira
        if (a2 == 'v' && b2 == 'i')
        {
            printf("    movl %d(%%rbp), %%r8d\n", posicao_variaveis[id2]);
            printf("    cmpl %%r8d, ");
        }
        //  registrador
        else if (a2 == 'v' && b2 == 'r')
        {
            printf("    cmpl %%r%dd,", posicao_variaveis[id2]);
        }
        //  constante
        else if (a2 == 'c' && b2 == 'i')
        {
            printf("    cmpl $%d, ", id2);
        }
        //  parametro
        else if (a2 == 'p' && b2 == 'i')
        {
            if (id2 == 1)
            {
                printf("    cmpl, %%edi ");
            }
            if (id2 == 2)
            {
                printf("    cmpl, %%esi ");
            }
            if (id2 == 3)
            {
                printf("    cmpl, %%edx ");
            }
        }

        // variavel inteira
        if (a1 == 'v' && b1 == 'i')
        {
            printf("%d(%%rbp)\n", posicao_variaveis[id1]);
        }
        //  registrador
        else if (a1 == 'v' && b1 == 'r')
        {
            printf("%%r%dd\n", posicao_variaveis[id1]);
        }
        //  constante
        else if (a1 == 'c' && b1 == 'i')
        {
            printf("$%d\n", id1);
        }
        //  parametro
        else if (a1 == 'p' && b1 == 'i')
        {
            if (id1 == 1)
            {
                printf("%%edi\n");
            }
            if (id1 == 2)
            {
                printf("%%esi\n");
            }
            if (id1 == 3)
            {
                printf("%%edx\n");
            }
        }

        comparacoes_if(c1, c2, (*qt_if));

        // comando
        while (fgets(line, LINESZ, stdin) != NULL)
        {
            remove_newline(line);

            if (strncmp(line, "endif", 3) == 0)
                break;

            // verifica se a linha é de atribuição de variável e, caso for, realiza a
            // operação
            atribuicao_de_variaveis(posicao_variaveis, line);

            // verifica se a linha é de set de array e, caso for, realiza a operação
            set_array(posicao_variaveis, line);

            // verifica se a linha é de get de array e, caso for, realiza a operação
            get_array(posicao_variaveis, line);

            // verifica se a linha é uma return
            retorna(posicao_variaveis, line, qtd_registradores, qtparam, tampilha);
        }

        printf("    end_if_%d:\n", (*qt_if));
        printf("    # ----------------------------------------------\n\n");
    }
}

int bloco_def_variaveis_e_alocacao_pilha(int posicao_variaveis[], char line[],
                                         int *tampilha, int qtparam)
{
    int qtd_registradores;
    // Verifica se a line começa com def
    if (strncmp(line, "def", 3) == 0)
    {
        // realiza as refinições de variáveis
        qtd_registradores = definicoes_de_variaveis(posicao_variaveis, line, tampilha, qtparam);

        (*tampilha) -= 8 * qtd_registradores;

        (*tampilha) -= 8 * qtparam;

        // verifica se o tamanho da pilha é múltiplo de 16, caso nao for,
        // transforme em um multiplo
        if (-(*tampilha) % 16 != 0)
            (*tampilha) -= (16 - (-(*tampilha)) % 16) % 16;

        // comentários registradores verdes
        int pos_verdes = (*tampilha) + 8 * (qtd_registradores + qtparam);
        if (qtd_registradores >= 1)
        {
            printf("    # r12 %d\n", pos_verdes - 8);
            pos_verdes -= 8;
        }
        if (qtd_registradores >= 2)
        {
            printf("    # r13 %d\n", pos_verdes - 8);
            pos_verdes -= 8;
        }
        if (qtd_registradores >= 3)
        {
            printf("    # r14 %d\n", pos_verdes - 8);
            pos_verdes -= 8;
        }
        if (qtd_registradores >= 4)
        {
            printf("    # r15 %d\n", pos_verdes - 8);
        }

        // alocação da pilha
        printf("    subq $%d, %%rsp\n\n", -(*tampilha));

        // salvamento dos registradores verdes
        pos_verdes = (*tampilha) + 8 * (qtd_registradores + qtparam);
        if (qtd_registradores >= 1)
        {
            printf("    movq %%r12, %d(%%rbp)\n", pos_verdes - 8);
            pos_verdes -= 8;
        }
        if (qtd_registradores >= 2)
        {
            printf("    movq %%r13, %d(%%rbp)\n", pos_verdes - 8);
            pos_verdes -= 8;
        }
        if (qtd_registradores >= 3)
        {
            printf("    movq %%r14, %d(%%rbp)\n", pos_verdes - 8);
            pos_verdes -= 8;
        }
        if (qtd_registradores >= 4)
        {
            printf("    movq %%r15, %d(%%rbp)\n", pos_verdes - 8);
        }
        if (qtd_registradores)
            printf("\n");
    }
    return qtd_registradores;
}

void atribuicao_de_variaveis(int posicao_variaveis[], char line[])
{
    int tipo, id1, id2, id3;
    char op, a1, b1, a2, b2, a3, b3;

    tipo = sscanf(line, "%c%c%d = %c%c%d %c %c%c%d", &a1, &b1, &id1, &a2, &b2,
                  &id2, &op, &a3, &b3, &id3);
    if (tipo == 10)
    {

        // salvando o 1° valor da operação em %r8d
        // variavel inteira
        if (a2 == 'v' && b2 == 'i')
        {
            printf("    movl %d(%%rbp), %%r8d\n", posicao_variaveis[id2]);
        }
        //  registrador
        else if (a2 == 'v' && b2 == 'r')
        {
            printf("    movl %%r%dd, %%r8d\n", posicao_variaveis[id2]);
        }
        //  constante
        else if (a2 == 'c' && b2 == 'i')
        {
            printf("    movl $%d, %%r8d\n", id2);
        }
        //  parametro
        else if (a2 == 'p' && b2 == 'i')
        {
            if (id2 == 1)
            {
                printf("    movl %%edi, %%r8d\n");
            }
            if (id2 == 2)
            {
                printf("    movl %%esi, %%r8d\n");
            }
            if (id2 == 3)
            {
                printf("    movl %%edx, %%r8d\n");
            }
        }
        // verifica a operação
        if (operation(op))
        {
            // variavel inteira
            if (a3 == 'v' && b3 == 'i')
            {
                printf("%d(%%rbp), %%r8d\n", posicao_variaveis[id3]);
            }
            //  registrador
            else if (a3 == 'v' && b3 == 'r')
            {
                printf("%%r%dd, %%r8d\n", posicao_variaveis[id3]);
            }
            //  constante
            else if (a3 == 'c' && b3 == 'i')
            {
                printf("$%d, %%r8d\n", id3);
            }
            //  parametro
            else if (a3 == 'p' && b3 == 'i')
            {
                if (id3 == 1)
                {
                    printf("%%edi, %%r8d\n");
                }
                if (id3 == 2)
                {
                    printf("%%esi, %%r8d\n");
                }
                if (id3 == 3)
                {
                    printf("%%edx, %%r8d\n");
                }
            }
        }
        else
        {
            // caso a operação for de divisão, salva o divisor no %r9d, dado que o
            // dividendo está no %r8d, a função divisao() é chamada para realizar a
            // operação variavel inteira
            if (a3 == 'v' && b3 == 'i')
            {
                printf("    movl %d(%%rbp), %%r9d\n", posicao_variaveis[id3]);
            }
            //  registrador
            else if (a3 == 'v' && b3 == 'r')
            {
                printf("    movl %%r%dd, %%r9d\n", posicao_variaveis[id3]);
            }
            //  constante
            else if (a3 == 'c' && b3 == 'i')
            {
                printf("    movl $%d, %%r9d\n", id3);
            }
            //  parametro
            else if (a3 == 'p' && b3 == 'i')
            {
                if (id3 == 1)
                {
                    printf("    movl %%edi, %%r9d\n");
                }
                if (id3 == 2)
                {
                    printf("    movl %%esi, %%r9d\n");
                }
                if (id3 == 3)
                {
                    printf("    movl %%edx, %%r9d\n");
                }
            }
            divisao();
        }
        // salvando resultado que está em %r8d para a variável a ser atribuída
        // variavel inteira
        if (a1 == 'v' && b1 == 'i')
        {
            printf("    movl %%r8d, %d(%%rbp)\n", posicao_variaveis[id1]);
        }
        //  registrador
        else if (a1 == 'v' && b1 == 'r')
        {
            printf("    movl %%r8d, %%r%dd\n", posicao_variaveis[id1]);
        }
        //  constante
        else if (a1 == 'c' && b1 == 'i')
        {
            printf("    movl %%r8d, $%d\n", id1);
        }
        //  parametro
        else if (a1 == 'p' && b1 == 'i')
        {
            if (id1 == 1)
            {
                printf("    movl %%r8d, %%edi\n");
            }
            if (id1 == 2)
            {
                printf("    movl %%r8d, %%esi\n");
            }
            if (id1 == 3)
            {
                printf("    movl %%r8d, %%edx\n");
            }
        }
        printf("\n");
        return;
    }

    //<<<<<<<<<<<<<<<
    tipo = sscanf(line, "vi%d = ci%d", &id1, &id2);
    if (tipo == 2)
    {
        // atribuir uma constante para uma váriavel.
        printf("    movl $%d, %d(%%rbp)\n\n", id2, posicao_variaveis[id1]);
        return;
    }

    tipo = sscanf(line, "vr%d = vi%d", &id1, &id2);
    if (tipo == 2)
    {
        // atribuir uma variavel para um registrador.
        printf("    movl %d(%%rbp), %%r%dd\n\n", posicao_variaveis[id2],
               posicao_variaveis[id1]);
        return;
    }

    tipo = sscanf(line, "vr%d = ci%d", &id1, &id2);
    if (tipo == 2)
    {
        // atribuir uma constante para um registrador.
        printf("    movl $%d, %%r%dd\n\n", id2, posicao_variaveis[id1]);
        return;
    }

    tipo = sscanf(line, "vi%d = vr%d", &id1, &id2);
    if (tipo == 2)
    {
        // atribuir um registrador para uma váriavel
        printf("    movl %%r%dd, %d(%%rbp)\n\n", posicao_variaveis[id2],
               posicao_variaveis[id1]);
        return;
    }

    tipo = sscanf(line, "vr%d = vr%d ", &id1, &id2);
    if (tipo == 2)
    {
        // atribuir um registrador para um registrador.
        printf("    movl %%r%dd, %%r%dd\n\n", posicao_variaveis[id2],
               posicao_variaveis[id1]);
        return;
    }
    tipo = sscanf(line, "vi%d = vi%d ", &id1, &id2);
    if (tipo == 2)
    {
        // atribuir um registrador para um registrador.
        printf("    movl %d(%%rbp), %%r8d\n", posicao_variaveis[id2]);
        printf("    movl %%r8d, %d(%%rbp)\n\n", posicao_variaveis[id1]);
        return;
    }
}

int definicoes_de_variaveis(int posicao_variaveis[], char line[], int *tampilha,
                            int qtparam)
{
    int cnt = 12;
    int qtd_registradores = 0;
    while (fgets(line, LINESZ, stdin) != NULL)
    {
        remove_newline(line);

        if (strncmp(line, "enddef", 6) == 0)
        {
            break;
        }
        int tipo, id1, tamanho_array;
        tipo = sscanf(line, "var vi%d", &id1);
        if (tipo == 1)
        { // ler a definição de uma váriavel inteira
            // printf("    subq $16, %%rsp\n");
            posicao_variaveis[id1] = (*tampilha) - 4;
            (*tampilha) -= 4;
            printf("    # vi%d %d\n", id1, (*tampilha));
            continue;
        }

        tipo = sscanf(line, "vet va%d size ci%d", &id1, &tamanho_array);
        if (tipo == 2)
        { // ler a definição de array
            // printf("    subq $%d, %%rsp\n", 16 * tamanho_array);
            posicao_variaveis[id1] = (*tampilha) - (4 * tamanho_array);
            (*tampilha) -= (4 * tamanho_array);
            printf("    # va%d %d\n", id1, (*tampilha));
            continue;
        }
        tipo = sscanf(line, "reg vr%d", &id1);
        if (tipo == 1)
        {                                   // ler a definição de um registrador
            qtd_registradores++;            // incrementa na quantidade de registrador
            posicao_variaveis[id1] = cnt++; // salvamos em qual registrador será
                                            // salva a variavel de registrador
            // caso auxliar == 1, atribuiremos o valor de reg1 em %r12d
            // caso auxliar == 2, atribuiremos o valor de reg1 em %r13d
            // caso auxliar == 3, atribuiremos o valor de reg1 em %r14d
            // caso auxliar == 4, atribuiremos o valor de reg1 em %r15d
            continue;
        }
    }
    return qtd_registradores;
}

void set_array(int posicao_variaveis[], char line[])
{
    int id2, id1, tipo, id;
    // atribuir um valor constante a um array local
    tipo = sscanf(line, "set va%d index ci%d with ci%d", &id1, &id, &id2);
    if (tipo == 3)
    {
        printf("    leaq %d(%%rbp), %%r8\n", posicao_variaveis[id1]);
        get_array_pos_i(id);
        printf("    movl $%d, (%%r8)\n\n", id2);
        return;
    }

    // atribuir uma variável local para um array
    tipo = sscanf(line, "set va%d index ci%d with vi%d", &id1, &id, &id2);
    if (tipo == 3)
    {
        printf("    leaq %d(%%rbp), %%r8\n", posicao_variaveis[id1]);
        get_array_pos_i(id);
        printf("    movl %d(%%rbp), %%r9d\n", posicao_variaveis[id2]);
        printf("    movl %%r9d, (%%r8)\n\n");
        return;
    }

    // atribuir um parâmetro inteiro para um array
    tipo = sscanf(line, "set va%d index ci%d with pi%d", &id1, &id, &id2);
    if (tipo == 3)
    {
        printf("    leaq %d(%%rbp), %%r8\n", posicao_variaveis[id1]);
        get_array_pos_i(id);
        if (id2 == 1)
        {
            printf("    movl %%edi, %%r9d\n");
        }
        if (id2 == 2)
        {
            printf("    movl %%esi, %%r9d\n");
        }
        if (id2 == 3)
        {
            printf("    movl %%edx, %%r9d\n");
        }
        printf("    movl %%r9d, (%%r8)\n\n");
        return;
    }

    // atribuir um registrador para um array local
    tipo = sscanf(line, "set va%d index ci%d with vr%d", &id1, &id, &id2);
    if (tipo == 3)
    {
        printf("    leaq %d(%%rbp), %%r8\n", posicao_variaveis[id1]);
        get_array_pos_i(id);
        printf("    movl %%r%dd, %%r9d\n", posicao_variaveis[id2]);
        printf("    movl %%r9d, (%%r8)\n\n");
        return;
    }

    // atribuir uma constante para um array passado pelo parâmetro da
    // função
    tipo = sscanf(line, "set pa%d index ci%d with ci%d", &id1, &id, &id2);
    if (tipo == 3)
    {
        if (id1 == 1)
        {
            printf("    movq %%rdi, %%r8\n");
        }
        if (id1 == 2)
        {
            printf("    movq %%rsi, %%r8\n");
        }
        if (id1 == 3)
        {
            printf("    movq %%rdx, %%r8\n");
        }
        get_array_pos_i(id);
        printf("    movl $%d, (%%r8)\n\n", id2);
        return;
    }

    // atribuir um valor de uma variável local para um array passado pelo
    // parâmetro da função
    tipo = sscanf(line, "set pa%d index ci%d with vi%d", &id1, &id, &id2);
    if (tipo == 3)
    {
        if (id1 == 1)
        {
            printf("    movq %%rdi, %%r8\n");
        }
        if (id1 == 2)
        {
            printf("    movq %%rsi, %%r8\n");
        }
        if (id1 == 3)
        {
            printf("    movq %%rdx, %%r8\n");
        }
        get_array_pos_i(id);
        printf("    movl %d(%%rbp), %%r9d\n", posicao_variaveis[id2]);
        printf("    movl %%r9d, (%%r8)\n\n");
        return;
    }

    // atribuir um parâmetro inteiro para um array passado pelo parâmetro da
    // função
    tipo = sscanf(line, "set pa%d index ci%d with pi%d", &id1, &id, &id2);
    if (tipo == 3)
    {
        if (id1 == 1)
        {
            printf("    movq %%rdi, %%r8\n");
        }
        if (id1 == 2)
        {
            printf("    movq %%rsi, %%r8\n");
        }
        if (id1 == 3)
        {
            printf("    movq %%rdx, %%r8\n");
        }
        get_array_pos_i(id);
        if (id2 == 1)
        {
            printf("    movl %%edi, %%r9d\n");
        }
        if (id2 == 2)
        {
            printf("    movl %%esi, %%r9d\n");
        }
        if (id2 == 3)
        {
            printf("    movl %%edx, %%r9d\n");
        }
        printf("    movl %%r9d, (%%r8)\n\n");
        return;
    }

    // atribuir um registrador para um array passado pelo parâmetro da função
    tipo = sscanf(line, "set pa%d index ci%d with vr%d", &id1, &id, &id2);
    if (tipo == 3)
    {
        if (id1 == 1)
        {
            printf("    movq %%rdi, %%r8\n");
        }
        if (id1 == 2)
        {
            printf("    movq %%rsi, %%r8\n");
        }
        if (id1 == 3)
        {
            printf("    movq %%rdx, %%r8\n");
        }
        get_array_pos_i(id);
        printf("    movl %%r%dd, %%r9d\n", posicao_variaveis[id2]);
        printf("    movl %%r9d, (%%r8)\n\n");
        return;
    }
}

void get_array(int posicao_variaveis[], char line[])
{
    int id2, id1, tipo, id;

    // atribuir o valor de um array para uma variável local
    tipo = sscanf(line, "get va%d index ci%d to vi%d", &id1, &id, &id2);
    if (tipo == 3)
    {
        printf("    leaq %d(%%rbp), %%r8\n", posicao_variaveis[id1]);
        get_array_pos_i(id);
        printf("    leaq %d(%%rbp), %%r9\n", posicao_variaveis[id2]);
        printf("    movl (%%r8), %%r10d\n");
        printf("    movl %%r10d, (%%r9)\n\n");
        return;
    }

    // atribuir o valor de um array para um parâmetro inteiro
    tipo = sscanf(line, "get va%d index ci%d to pi%d", &id1, &id, &id2);
    if (tipo == 3)
    {
        printf("    leaq %d(%%rbp), %%r8\n", posicao_variaveis[id1]);
        get_array_pos_i(id);
        if (id2 == 1)
        {
            printf("    movl (%%r8), %%edi\n\n");
        }
        if (id2 == 2)
        {
            printf("    movl (%%r8), %%esi\n\n");
        }
        if (id2 == 3)
        {
            printf("    movl (%%r8), %%edx\n\n");
        }
        return;
    }

    // atribuir o valor de um array local para um registrador
    tipo = sscanf(line, "get va%d index ci%d to vr%d", &id1, &id, &id2);
    if (tipo == 3)
    {
        printf("    leaq %d(%%rbp), %%r8\n", posicao_variaveis[id1]);
        get_array_pos_i(id);
        printf("    movl (%%r8), %%r%dd\n\n", posicao_variaveis[id2]);
        return;
    }

    // atribuir um valor de um array passado pelo parâmetro da função para uma
    // variável local
    tipo = sscanf(line, "get pa%d index ci%d to vi%d", &id1, &id, &id2);
    if (tipo == 3)
    {
        if (id1 == 1)
        {
            printf("    movq %%rdi, %%r8\n");
        }
        if (id1 == 2)
        {
            printf("    movq %%rsi, %%r8\n");
        }
        if (id1 == 3)
        {
            printf("    movq %%rdx, %%r8\n");
        }
        get_array_pos_i(id);
        printf("    leaq %d(%%rbp), %%r9\n", posicao_variaveis[id2]);
        printf("    movl (%%r8), %%r10d\n");
        printf("    movl %%r10d, (%%r9)\n\n");
        return;
    }

    // atribuir o valor de um array passado pelo parâmetro da função para um
    // parâmetro inteiro
    tipo = sscanf(line, "get pa%d index ci%d to pi%d", &id1, &id, &id2);
    if (tipo == 3)
    {
        if (id1 == 1)
        {
            printf("    movq %%rdi, %%r8\n");
        }
        if (id1 == 2)
        {
            printf("    movq %%rsi, %%r8\n");
        }
        if (id1 == 3)
        {
            printf("    movq %%rdx, %%r8\n");
        }
        get_array_pos_i(id);
        if (id2 == 1)
        {
            printf("    movl (%%r8), %%edi\n\n");
        }
        if (id2 == 2)
        {
            printf("    movl (%%r8), %%esi\n\n");
        }
        if (id2 == 3)
        {
            printf("    movl (%%r8), %%edx\n\n");
        }
        return;
    }

    // atribuir o valor de um array passado pelo parâmetro da função para um
    // registrador
    tipo = sscanf(line, "get pa%d index ci%d to vr%d", &id1, &id, &id2);
    if (tipo == 3)
    {
        if (id1 == 1)
        {
            printf("    movq %%rdi, %%r8\n");
        }
        if (id1 == 2)
        {
            printf("    movq %%rsi, %%r8\n");
        }
        if (id1 == 3)
        {
            printf("    movq %%rdx, %%r8\n");
        }
        get_array_pos_i(id);
        printf("    movl (%%r8), %%r%ds\n\n", posicao_variaveis[id2]);
        return;
    }
}

void call(int posicao_variaveis[], char line[], int qtd_registradores,
          int qtparam, int tampilha)
{
    int param_number; // número de parâmetros da função chamada
    int fun, p1, p2, p3, receive, pos;
    char ctr1, ctr2, ctr3;
    char t1, t2, t3;
    pos = tampilha + 8 * qtparam;
    param_number = sscanf(line, "vi%d = call f%d %c%c%d %c%c%d %c%c%d", &receive, &fun, &t1,
                          &ctr1, &p1, &t2, &ctr2, &p2, &t3, &ctr3, &p3);
    // verifica e resolve para funções com 3 parametros que retornam em uma
    // váriavel;
    if (param_number == 11)
    {
        salva_parametros(pos, qtparam);
        if (t1 == 'v')
        {
            if (ctr1 == 'r')
            {
                printf("    movl %%r%dd, %%edi", posicao_variaveis[p1]);
            }
            if (ctr1 == 'a')
            {
                printf("    leaq %d(%%rbp), %%rdi", posicao_variaveis[p1]);
            }
            if (ctr1 == 'i')
            {
                printf("    movl %d(%%rbp), %%edi", posicao_variaveis[p1]);
            }
        }
        else if (t1 == 'p')
        {
            if (p1 == 1)
                printf("    movl %d(%%rbp), %%edi", pos - 8);
            if (p1 == 2)
                printf("    movl %d(%%rbp), %%edi", pos - 16);
            if (p1 == 3)
                printf("    movl %d(%%rbp), %%edi", pos - 24);
        }
        else
        {
            printf("    movl $%d, %%edi", p1);
        }
        printf("\n");

        if (t2 == 'v')
        {
            if (ctr2 == 'r')
            {
                printf("    movl %%r%dd, %%esi", posicao_variaveis[p2]);
            }
            if (ctr2 == 'a')
            {
                printf("    leaq %d(%%rbp), %%rsi", posicao_variaveis[p2]);
            }
            if (ctr2 == 'i')
            {
                printf("    movl %d(%%rbp), %%esi", posicao_variaveis[p2]);
            }
        }
        else if (t2 == 'p')
        {
            if (p2 == 1)
                printf("    movl %d(%%rbp), %%esi", pos - 8);
            if (p2 == 2)
                printf("    movl %d(%%rbp), %%esi", pos - 16);
            if (p2 == 3)
                printf("    movl %d(%%rbp), %%esi", pos - 24);
        }
        else
        {
            printf("    movl $%d, %%esi", p2);
        }
        printf("\n");

        if (t3 == 'v')
        {
            if (ctr3 == 'r')
            {
                printf("    movl %%r%dd, %%edx", posicao_variaveis[p3]);
            }
            if (ctr3 == 'a')
            {
                printf("    leaq %d(%%rbp), %%rdx", posicao_variaveis[p3]);
            }
            if (ctr3 == 'i')
            {
                printf("    movl %d(%%rbp), %%edx", posicao_variaveis[p3]);
            }
        }
        else if (t3 == 'p')
        {
            if (p3 == 1)
                printf("    movl %d(%%rbp), %%edx", pos - 8);
            if (p3 == 2)
                printf("    movl %d(%%rbp), %%edx", pos - 16);
            if (p3 == 3)
                printf("    movl %d(%%rbp), %%edx", pos - 24);
        }
        else
        {
            printf("    movl $%d, %%edx", p3);
        }
        printf("\n");
        printf("    call f%d\n", fun);
        printf("    movl %%eax, %d(%%rbp)\n\n", posicao_variaveis[receive]);
        recupera_parametros(pos, qtparam);
        return;
    }
    param_number = sscanf(line, "vi%d = call f%d %c%c%d %c%c%d", &receive, &fun,
                          &t1, &ctr1, &p1, &t2, &ctr2, &p2);
    // verifica e resolve para funções com 2 parametros que retornam em uma
    // váriavel;
    if (param_number == 8)
    {
        salva_parametros(pos, qtparam);
        if (t1 == 'v')
        {
            if (ctr1 == 'r')
            {
                printf("    movl %%r%dd, %%edi", posicao_variaveis[p1]);
            }
            if (ctr1 == 'a')
            {
                printf("    leaq %d(%%rbp), %%rdi", posicao_variaveis[p1]);
            }
            if (ctr1 == 'i')
            {
                printf("    movl %d(%%rbp), %%edi", posicao_variaveis[p1]);
            }
        }
        else if (t1 == 'p')
        {
            if (p1 == 1)
                printf("    movl %d(%%rbp), %%edi", pos - 8);
            if (p1 == 2)
                printf("    movl %d(%%rbp), %%edi", pos - 16);
            if (p1 == 3)
                printf("    movl %d(%%rbp), %%edi", pos - 24);
        }
        else
        {
            printf("    movl $%d, %%edi", p1);
        }
        printf("\n");

        if (t2 == 'v')
        {
            if (ctr2 == 'r')
            {
                printf("    movl %%r%dd, %%esi", posicao_variaveis[p2]);
            }
            if (ctr2 == 'a')
            {
                printf("    leaq %d(%%rbp), %%rsi", posicao_variaveis[p2]);
            }
            if (ctr2 == 'i')
            {
                printf("    movl %d(%%rbp), %%esi", posicao_variaveis[p2]);
            }
        }
        else if (t2 == 'p')
        {
            if (p2 == 1)
                printf("    movl %d(%%rbp), %%esi", pos - 8);
            if (p2 == 2)
                printf("    movl %d(%%rbp), %%esi", pos - 16);
            if (p2 == 3)
                printf("    movl %d(%%rbp), %%esi", pos - 24);
        }
        else
        {
            printf("    movl $%d, %%esi", p2);
        }
        printf("\n");

        printf("    call f%d\n", fun);
        printf("    movl %%eax, %d(%%rbp)\n\n", posicao_variaveis[receive]);
        recupera_parametros(pos, qtparam);
        return;
    }

    param_number = sscanf(line, "vi%d = call f%d %c%c%d", &receive, &fun, &t1, &ctr1, &p1);
    // verifica e resolve para funções com 1 parametros que retornam em uma
    // váriavel;
    if (param_number == 5)
    {
        salva_parametros(pos, qtparam);
        if (t1 == 'v')
        {
            if (ctr1 == 'r')
            {
                printf("    movl %%r%dd, %%edi", posicao_variaveis[p1]);
            }
            if (ctr1 == 'a')
            {
                printf("    leaq %d(%%rbp), %%rdi", posicao_variaveis[p1]);
            }
            if (ctr1 == 'i')
            {
                printf("    movl %d(%%rbp), %%edi", posicao_variaveis[p1]);
            }
        }
        else if (t1 == 'p')
        {
            if (p1 == 1)
                printf("    movl %d(%%rbp), %%edi", pos - 8);
            if (p1 == 2)
                printf("    movl %d(%%rbp), %%edi", pos - 16);
            if (p1 == 3)
                printf("    movl %d(%%rbp), %%edi", pos - 24);
        }
        else
        {
            printf("    movl $%d, %%edi", p1);
        }
        printf("\n");
        printf("    call f%d\n", fun);
        printf("    movl %%eax, %d(%%rbp)\n\n", posicao_variaveis[receive]);
        recupera_parametros(pos, qtparam);
        return;
    }

    param_number = sscanf(line, "vi%d = call f%d", &receive, &fun);
    // verifica e resolve para funções com 0 parametros que retornam em uma
    // váriavel;
    if (param_number == 2)
    {
        salva_parametros(pos, qtparam);
        printf("    call f%d\n", fun);
        printf("    movl %%eax, %d(%%rbp)\n\n", posicao_variaveis[receive]);
        recupera_parametros(pos, qtparam);
        return;
    }

    param_number = sscanf(line, "vr%d = call f%d %c%c%d %c%c%d %c%c%d", &receive, &fun, &t1,
                          &ctr1, &p1, &t2, &ctr2, &p2, &t3, &ctr3, &p3);
    // verifica e resolve para funções com 3 parametros que retornam em um
    // registrador;
    if (param_number == 11)
    {
        salva_parametros(pos, qtparam);
        if (t1 == 'v')
        {
            if (ctr1 == 'r')
            {
                printf("    movl %%r%dd, %%edi", posicao_variaveis[p1]);
            }
            if (ctr1 == 'a')
            {
                printf("    leaq %d(%%rbp), %%rdi", posicao_variaveis[p1]);
            }
            if (ctr1 == 'i')
            {
                printf("    movl %d(%%rbp), %%edi", posicao_variaveis[p1]);
            }
        }
        else if (t1 == 'p')
        {
            if (p1 == 1)
                printf("    movl %d(%%rbp), %%edi", pos - 8);
            if (p1 == 2)
                printf("    movl %d(%%rbp), %%edi", pos - 16);
            if (p1 == 3)
                printf("    movl %d(%%rbp), %%edi", pos - 24);
        }
        else
        {
            printf("    movl $%d, %%edi", p1);
        }
        printf("\n");
        if (t2 == 'v')
        {
            if (ctr2 == 'r')
            {
                printf("    movl %%r%dd, %%esi", posicao_variaveis[p2]);
            }
            if (ctr2 == 'a')
            {
                printf("    leaq %d(%%rbp), %%rsi", posicao_variaveis[p2]);
            }
            if (ctr2 == 'i')
            {
                printf("    movl %d(%%rbp), %%esi", posicao_variaveis[p2]);
            }
        }
        else if (t2 == 'p')
        {
            if (p2 == 1)
                printf("    movl %d(%%rbp), %%esi", pos - 8);
            if (p2 == 2)
                printf("    movl %d(%%rbp), %%esi", pos - 16);
            if (p2 == 3)
                printf("    movl %d(%%rbp), %%esi", pos - 24);
        }
        else
        {
            printf("    movl $%d, %%esi", p2);
        }
        printf("\n");
        if (t3 == 'v')
        {
            if (ctr3 == 'r')
            {
                printf("    movl %%r%dd, %%edx", posicao_variaveis[p3]);
            }
            if (ctr3 == 'a')
            {
                printf("    leaq %d(%%rbp), %%rdx", posicao_variaveis[p3]);
            }
            if (ctr3 == 'i')
            {
                printf("    movl %d(%%rbp), %%edx", posicao_variaveis[p3]);
            }
        }
        else if (t3 == 'p')
        {
            if (p3 == 1)
                printf("    movl %d(%%rbp), %%edx", pos - 8);
            if (p3 == 2)
                printf("    movl %d(%%rbp), %%edx", pos - 16);
            if (p3 == 3)
                printf("    movl %d(%%rbp), %%edx", pos - 24);
        }
        else
        {
            printf("    movl $%d, %%edx", p3);
        }
        printf("\n");
        printf("    call f%d\n", fun);
        printf("    movl %%eax, %%r%dd\n\n", posicao_variaveis[receive]);
        recupera_parametros(pos, qtparam);
        return;
    }
    param_number = sscanf(line, "vr%d = call f%d %c%c%d %c%c%d", &receive, &fun,
                          &t1, &ctr1, &p1, &t2, &ctr2, &p2);
    // verifica e resolve para funções com 2 parametros que retornam em um
    // registrador;
    if (param_number == 8)
    {
        salva_parametros(pos, qtparam);
        if (t1 == 'v')
        {
            if (ctr1 == 'r')
            {
                printf("    movl %%r%dd, %%edi", posicao_variaveis[p1]);
            }
            if (ctr1 == 'a')
            {
                printf("    leaq %d(%%rbp), %%rdi", posicao_variaveis[p1]);
            }
            if (ctr1 == 'i')
            {
                printf("    movl %d(%%rbp), %%edi", posicao_variaveis[p1]);
            }
        }
        else if (t1 == 'p')
        {
            if (p1 == 1)
                printf("    movl %d(%%rbp), %%edi", pos - 8);
            if (p1 == 2)
                printf("    movl %d(%%rbp), %%edi", pos - 16);
            if (p1 == 3)
                printf("    movl %d(%%rbp), %%edi", pos - 24);
        }
        else
        {
            printf("    movl $%d, %%edi", p1);
        }
        printf("\n");
        if (t2 == 'v')
        {
            if (ctr2 == 'r')
            {
                printf("    movl %%r%dd, %%esi", posicao_variaveis[p2]);
            }
            if (ctr2 == 'a')
            {
                printf("    leaq %d(%%rbp), %%rsi", posicao_variaveis[p2]);
            }
            if (ctr2 == 'i')
            {
                printf("    movl %d(%%rbp), %%esi", posicao_variaveis[p2]);
            }
        }
        else if (t2 == 'p')
        {
            if (p2 == 1)
                printf("    movl %d(%%rbp), %%esi", pos - 8);
            if (p2 == 2)
                printf("    movl %d(%%rbp), %%esi", pos - 16);
            if (p2 == 3)
                printf("    movl %d(%%rbp), %%esi", pos - 24);
        }
        else
        {
            printf("    movl $%d, %%esi", p2);
        }
        printf("\n");
        printf("    call f%d\n", fun);
        printf("    movl %%eax, %%r%dd\n\n", posicao_variaveis[receive]);
        recupera_parametros(pos, qtparam);
        return;
    }

    param_number = sscanf(line, "vr%d = call f%d %c%c%d", &receive, &fun, &t3, &ctr1, &p1);
    // verifica e resolve para funções com 1 parametros que retornam em um
    // registrador;
    if (param_number == 5)
    {
        salva_parametros(pos, qtparam);
        if (t1 == 'v')
        {
            if (ctr1 == 'r')
            {
                printf("    movl %%r%dd, %%edi", posicao_variaveis[p1]);
            }
            if (ctr1 == 'a')
            {
                printf("    leaq %d(%%rbp), %%rdi", posicao_variaveis[p1]);
            }
            if (ctr1 == 'i')
            {
                printf("    movl %d(%%rbp), %%edi", posicao_variaveis[p1]);
            }
        }
        else if (t1 == 'p')
        {
            if (p1 == 1)
                printf("    movl %d(%%rbp), %%edi", pos - 8);
            if (p1 == 2)
                printf("    movl %d(%%rbp), %%edi", pos - 16);
            if (p1 == 3)
                printf("    movl %d(%%rbp), %%edi", pos - 24);
        }
        else
        {
            printf("    movl $%d, %%edi", p1);
        }
        printf("\n");
        printf("    call f%d\n", fun);
        printf("    movl %%eax, %%r%dd\n\n", posicao_variaveis[receive]);
        recupera_parametros(pos, qtparam);
        return;
    }

    param_number = sscanf(line, "vr%d = call f%d", &receive, &fun);
    // verifica e resolve para funções com 0 parametros que retornam em um
    // registrador;
    if (param_number == 2)
    {
        salva_parametros(pos, qtparam);
        printf("    call f%d\n", fun);
        printf("    movl %%eax, %%r%dd\n\n", posicao_variaveis[receive]);
        recupera_parametros(pos, qtparam);
        return;
    }

    param_number = sscanf(line, "pi%d = call f%d %c%c%d %c%c%d %c%c%d", &receive, &fun, &t1,
                          &ctr1, &p1, &t2, &ctr2, &p2, &t3, &ctr3, &p3);
    // verifica e resolve para funções com 3 parametros que retornam em um
    // parametro;
    if (param_number == 11)
    {
        salva_parametros(pos, qtparam);
        if (t1 == 'v')
        {
            if (ctr1 == 'r')
            {
                printf("    movl %%r%dd, %%edi", posicao_variaveis[p1]);
            }
            if (ctr1 == 'a')
            {
                printf("    leaq %d(%%rbp), %%rdi", posicao_variaveis[p1]);
            }
            if (ctr1 == 'i')
            {
                printf("    movl %d(%%rbp), %%edi", posicao_variaveis[p1]);
            }
        }
        else if (t1 == 'p')
        {
            if (p1 == 1)
                printf("    movl %d(%%rbp), %%edi", pos - 8);
            if (p1 == 2)
                printf("    movl %d(%%rbp), %%edi", pos - 16);
            if (p1 == 3)
                printf("    movl %d(%%rbp), %%edi", pos - 24);
        }
        else
        {
            printf("    movl $%d, %%edi", p1);
        }
        printf("\n");
        if (t2 == 'v')
        {
            if (ctr2 == 'r')
            {
                printf("    movl %%r%dd, %%esi", posicao_variaveis[p2]);
            }
            if (ctr2 == 'a')
            {
                printf("    leaq %d(%%rbp), %%rsi", posicao_variaveis[p2]);
            }
            if (ctr2 == 'i')
            {
                printf("    movl %d(%%rbp), %%esi", posicao_variaveis[p2]);
            }
        }
        else if (t2 == 'p')
        {
            if (p2 == 1)
                printf("    movl %d(%%rbp), %%esi", pos - 8);
            if (p2 == 2)
                printf("    movl %d(%%rbp), %%esi", pos - 16);
            if (p2 == 3)
                printf("    movl %d(%%rbp), %%esi", pos - 24);
        }
        else
        {
            printf("    movl $%d, %%esi", p2);
        }
        printf("\n");
        if (t3 == 'v')
        {
            if (ctr3 == 'r')
            {
                printf("    movl %%r%dd, %%edx", posicao_variaveis[p3]);
            }
            if (ctr3 == 'a')
            {
                printf("    leaq %d(%%rbp), %%rdx", posicao_variaveis[p3]);
            }
            if (ctr3 == 'i')
            {
                printf("    movl %d(%%rbp), %%edx", posicao_variaveis[p3]);
            }
        }
        else if (t3 == 'p')
        {
            if (p3 == 1)
                printf("    movl %d(%%rbp), %%edx", pos - 8);
            if (p3 == 2)
                printf("    movl %d(%%rbp), %%edx", pos - 16);
            if (p3 == 3)
                printf("    movl %d(%%rbp), %%edx", pos - 24);
        }
        else
        {
            printf("    movl $%d, %%edx", p3);
        }
        printf("\n");
        printf("    call f%d\n", fun);
        recupera_parametros(pos, qtparam);
        printf("    movl %%eax,");
        if (receive == 1)
            printf("%%edi\n");
        if (receive == 2)
            printf("%%esi\n");
        if (receive == 3)
            printf("%%edx\n");
        printf("\n");

        return;
    }

    param_number = sscanf(line, "pi%d = call f%d %c%c%d %c%c%d", &receive, &fun,
                          &t1, &ctr1, &p1, &t2, &ctr2, &p2);
    // verifica e resolve para funções com 2 parametros que retornam em um
    // parametro;
    if (param_number == 8)
    {
        salva_parametros(pos, qtparam);
        if (t1 == 'v')
        {
            if (ctr1 == 'r')
            {
                printf("    movl %%r%dd, %%edi", posicao_variaveis[p1]);
            }
            if (ctr1 == 'a')
            {
                printf("    leaq %d(%%rbp), %%rdi", posicao_variaveis[p1]);
            }
            if (ctr1 == 'i')
            {
                printf("    movl %d(%%rbp), %%edi", posicao_variaveis[p1]);
            }
        }
        else if (t1 == 'p')
        {
            if (p1 == 1)
                printf("    movl %d(%%rbp), %%edi", pos - 8);
            if (p1 == 2)
                printf("    movl %d(%%rbp), %%edi", pos - 16);
            if (p1 == 3)
                printf("    movl %d(%%rbp), %%edi", pos - 24);
        }
        else
        {
            printf("    movl $%d, %%edi", p1);
        }
        printf("\n");
        if (t2 == 'v')
        {
            if (ctr2 == 'r')
            {
                printf("    movl %%r%dd, %%esi", posicao_variaveis[p2]);
            }
            if (ctr2 == 'a')
            {
                printf("    leaq %d(%%rbp), %%rsi", posicao_variaveis[p2]);
            }
            if (ctr2 == 'i')
            {
                printf("    movl %d(%%rbp), %%esi", posicao_variaveis[p2]);
            }
        }
        else if (t2 == 'p')
        {
            if (p2 == 1)
                printf("    movl %d(%%rbp), %%esi", pos - 8);
            if (p2 == 2)
                printf("    movl %d(%%rbp), %%esi", pos - 16);
            if (p2 == 3)
                printf("    movl %d(%%rbp), %%esi", pos - 24);
        }
        else
        {
            printf("    movl $%d, %%esi", p2);
        }
        printf("\n");
        printf("    call f%d\n", fun);
        recupera_parametros(pos, qtparam);
        printf("    movl %%eax, ");
        if (receive == 1)
            printf("%%edi\n");
        if (receive == 2)
            printf("%%esi\n");
        if (receive == 3)
            printf("%%edx\n");
        printf("\n");
        return;
    }

    param_number = sscanf(line, "pi%d = call f%d %c%c%d", &receive, &fun, &t1, &ctr1, &p1);
    // verifica e resolve para funções com 1 parametros que retornam em um
    // parametro;
    if (param_number == 5)
    {
        salva_parametros(pos, qtparam);
        if (t1 == 'v')
        {
            if (ctr1 == 'r')
            {
                printf("    movl %%r%dd, %%edi", posicao_variaveis[p1]);
            }
            if (ctr1 == 'a')
            {
                printf("    leaq %d(%%rbp), %%rdi", posicao_variaveis[p1]);
            }
            if (ctr1 == 'i')
            {
                printf("    movl %d(%%rbp), %%edi", posicao_variaveis[p1]);
            }
        }
        else if (t1 == 'p')
        {
            if (p1 == 1)
                printf("    movl %d(%%rbp), %%edi", pos - 8);
            if (p1 == 2)
                printf("    movl %d(%%rbp), %%edi", pos - 16);
            if (p1 == 3)
                printf("    movl %d(%%rbp), %%edi", pos - 24);
        }
        else
        {
            printf("    movl $%d, %%edi", p1);
        }
        printf("\n");
        printf("    call f%d\n", fun);
        recupera_parametros(pos, qtparam);
        printf("    movl %%eax, ");
        if (receive == 1)
            printf("%%edi\n");
        if (receive == 2)
            printf("%%esi\n");
        if (receive == 3)
            printf("%%edx\n");
        printf("\n");
        return;
    }

    param_number = sscanf(line, "pi%d = call f%d", &receive, &fun);
    // verifica e resolve para funções com 0 parametros que retornam em um
    // parametro;
    if (param_number == 2)
    {
        salva_parametros(pos, qtparam);
        printf("    call f%d\n", fun);
        recupera_parametros(pos, qtparam);
        printf("    movl %%eax, ");
        if (receive == 1)
            printf("%%edi\n");
        if (receive == 2)
            printf("%%esi\n");
        if (receive == 3)
            printf("%%edx\n");
        printf("\n");
        return;
    }
}
void end_code(int tampilha, int qtd_registradores, int qtparam)
{
    int pos_verdes = tampilha + 8 * (qtd_registradores + qtparam);
    if (qtd_registradores >= 1)
    {
        printf("    movq %d(%%rbp), %%r12\n", pos_verdes - 8);
        pos_verdes -= 8;
    }
    if (qtd_registradores >= 2)
    {
        printf("    movq %d(%%rbp), %%r13\n", pos_verdes - 8);
        pos_verdes -= 8;
    }
    if (qtd_registradores >= 3)
    {
        printf("    movq %d(%%rbp), %%r14\n", pos_verdes - 8);
        pos_verdes -= 8;
    }
    if (qtd_registradores >= 4)
    {
        printf("    movq %d(%%rbp), %%r15\n", pos_verdes - 8);
    }
    if (qtd_registradores)
        printf("\n");

    // fim do código
    printf("    leave\n");
    printf("    ret\n\n");
}
void retorna(int posicao_variaveis[], char line[], int qtd_registradores,
             int qtparam, int tampilha)
{
    int param_number, value;
    param_number = sscanf(line, "return ci%d", &value);
    if (param_number == 1)
    {
        printf("    movl $%d, %%eax\n\n", value);
        end_code(tampilha, qtd_registradores, qtparam);
        return;
    }
    param_number = sscanf(line, "return vi%d", &value);
    if (param_number == 1)
    {
        printf("    movl %d(%%rbp), %%eax\n\n", posicao_variaveis[value]);
        end_code(tampilha, qtd_registradores, qtparam);
        return;
    }
    param_number = sscanf(line, "return pi%d", &value);
    if (param_number == 1)
    {
        if (value == 1)
        {
            printf("    movl %%edi");
        }
        if (value == 2)
        {
            printf("    movl %%esi");
        }
        if (value == 3)
        {
            printf("    movl %%edx");
        }
        printf(",%%eax\n\n");
        end_code(tampilha, qtd_registradores, qtparam);
        return;
    }
}

void def_f(int fi, int qtparam)
{
    // variável que controlará o tamanho da alocação da pilha em bytes
    int tampilha = 0;

    // essa variável salvará a posicao inicial da pilha onde os parametros das
    // funcoes serão salvos quando houver chamadas de funcoes
    int posicao_pilha_param;

    // em posição_variaveis[i], o i representa o indice da variavel e o valor do
    // vetor representa o lugar que está na pilha. Caso a variável local for de
    // registrados, o valor do vetor pode assumir os valores 12, 13, 14 e 15,
    // representando a variavel com indice i pode estar salva em %r12d, %r13d,
    // %r14d e %r15d respectivamente
    int posicao_variaveis[13];

    // mark_definicoes_variaveis é um booleano para indicar se o bloco de
    // definicoes de variaveis já ocorreu, 1 caso já tenha ocorrido, 0 caso
    // contrário
    int mark_definicoes_variaveis = 0, qtd_registradores, qt_if = 0;
    long long id;
    char line[LINESZ]; // string da linha
    // declaração da função
    printf(".globl f%d\n", fi);
    printf("f%d:\n", fi);

    // move %rbp e %rsp para a RA da função chamadora
    printf("    pushq %%rbp\n");
    printf("    movq %%rsp, %%rbp\n\n");

    while (fgets(line, LINESZ, stdin) != NULL)
    {
        remove_newline(line);

        if (!mark_definicoes_variaveis)
        {
            qtd_registradores = bloco_def_variaveis_e_alocacao_pilha(
                posicao_variaveis, line, &tampilha, qtparam);
            mark_definicoes_variaveis ^= 1;
            continue;
        }
        // Verifica se line começa com 'end'
        if (strncmp(line, "end", 3) == 0)
        {
            // recuperação dos registradores verdes
            return;
        }
        // verifica se a linha é de atribuição de variável e, caso for, realiza a
        // operação
        atribuicao_de_variaveis(posicao_variaveis, line);

        // verifica se a linha é de set de array e, caso for, realiza a operação
        set_array(posicao_variaveis, line);

        // verifica se a linha é de get de array e, caso for, realiza a operação
        get_array(posicao_variaveis, line);

        // verifica se a linha é um call
        call(posicao_variaveis, line, qtd_registradores, qtparam, tampilha);

        // verifica se a linha é uma return
        retorna(posicao_variaveis, line, qtd_registradores, qtparam, tampilha);

        // verifica se a linha é de condicional
        condicional(posicao_variaveis, line, &qt_if, qtd_registradores, qtparam,
                    tampilha);
    }
}

int main()
{

    printf(".data\n\n.text\n\n"); // início do código em Assembly

    char v1;
    int r, i;
    int inicio, fim, passo;
    char line[LINESZ];
    char ch1, ch2, ch3;
    int cnt1, cnt2, cnt3;

    // Lê uma linha por vez
    while (fgets(line, LINESZ, stdin) != NULL)
    {
        remove_newline(line);
        if (strncmp(line, "end", 3) == 0)
        {
            printf("\n");
            continue;
        }
        // lendo uma função que possui até 3 parâmetros
        r = sscanf(line, "function f%d p%c%d p%c%d p%c%d", &i, &ch1, &cnt1, &ch2,
                   &cnt2, &ch3, &cnt3);
        if (r == 7)
        {
            def_f(i, r / 2);
            continue;
        }
        r = sscanf(line, "function f%d p%c%d p%c%d", &i, &ch1, &cnt1, &ch2, &cnt2);
        if (r == 5)
        {
            def_f(i, r / 2);
            continue;
        }
        r = sscanf(line, "function f%d p%c%d", &i, &ch1, &cnt1);
        if (r == 3)
        {
            def_f(i, r / 2);
            continue;
        }
        r = sscanf(line, "function f%d", &i);
        if (r == 1)
        {
            def_f(i, r / 2);
            continue;
        }
    }

    return 0;
}